#include <algorithm>
#include <bitset>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <tuple>
#include <utility>
#include <vector>
#include <stack>
#include <ctime>
#include <random>
#include <iomanip>
#include <sstream>
#include "state/state.h"
#include "state/state_impl.h"


state_impl::state_impl() {
  nodes.push_back(state_impl::node());
}

std::string state_impl::get(std::string key) {
  int32_t node_index = get_node_index(key);
  std::cout << "index at get for key " << key << " :"
      << node_index << std::endl;
  return node_index == -1 ? "" : nodes[node_index].value;
}
void state_impl::set(std::string key, std::string value) {
  if (value == "") {
    erase(key);
    return;
  }
  uint32_t cur_node = 0;
  uint32_t cur_prefix_index = 0;
  unsigned int i = 0;
  for (; i < key.length(); ++i) {
    unsigned char path_element = key[i];
    // If there is no prefix or thre is prefix but we have reached the end
    // TODO(Samir): Check why this can't be just
    // cur_prefix_index == nodes[cur_node].prefix.length()
    if (nodes[cur_node].prefix.length() == 0 || (cur_prefix_index != 0
        && cur_prefix_index == nodes[cur_node].prefix.length())) {
      // If there is children with the next path element continue on the path
      if (nodes[cur_node].children[path_element] != 0) {
        cur_node = nodes[cur_node].children[path_element];
      // This node has children, value set or is the root
      } else if (has_children(cur_node) ||
          nodes[cur_node].value != "" || cur_node == 0) {
        cur_node = add_node(cur_node, path_element);
      // if the remaining key is more than a single char
      } else {
        // this node has no children so it's the final node
        // and the remainder of the path will be the prefix
        nodes[cur_node].prefix = key.substr(i);
        // cur_prefix_index = nodes[cur_node].prefix.length();
        break;
      }
      cur_prefix_index = 0;
    } else {
      // If there is prefix at this node -> progress prefix
      // If next path element does not match the next prefix element,
      // create a split node at the difference
      if (path_element != nodes[cur_node].prefix[cur_prefix_index]) {
        const std::string cur_node_prefix = nodes[cur_node].prefix;

        // Create the split_node and set up links with cur_node
        uint32_t split_node = add_node(nodes[cur_node].parent,
            key[i-cur_prefix_index-1]);
        // Set split node as parent of cur_node
        nodes[cur_node].parent = split_node;
        // Set the current node as child of the new split node
        nodes[split_node].children[cur_node_prefix[cur_prefix_index]]=cur_node;

        // set prefix of split_node and cur_node
        if (cur_prefix_index >= 1) {
          nodes[split_node].prefix = cur_node_prefix.substr(0,
              cur_prefix_index);
        }
        if (cur_node_prefix.length() - cur_prefix_index > 1) {
          nodes[cur_node].prefix = cur_node_prefix.substr(cur_prefix_index+1);
        } else {
          nodes[cur_node].prefix = "";
        }
        // Create the new node from the split to the next path element
        cur_node = add_node(split_node, path_element);
        cur_prefix_index = 0;
      } else {
        ++cur_prefix_index;
      }
    }
  }
  if ((nodes[cur_node].prefix.length() != cur_prefix_index)
      && i == key.length()) {
    // We are at the last key element and there is still prefix left.
    // Create node here
    const std::string cur_node_prefix = nodes[cur_node].prefix;

    // Create the split_node and set up links with cur_node
    uint32_t split_node = add_node(nodes[cur_node].parent, key[key.length() -
        cur_prefix_index-1]);
    // Set split node as parent of cur_node
    nodes[cur_node].parent = split_node;
    // Set the current node as child of the new split node
    nodes[split_node].children[cur_node_prefix[cur_prefix_index]] = cur_node;

    // set prefix of split_node and cur_node
    if (cur_prefix_index >= 1) {
      nodes[split_node].prefix = cur_node_prefix.substr(0, cur_prefix_index);
    }
    if (cur_node_prefix.length() - cur_prefix_index > 1) {
      nodes[cur_node].prefix = cur_node_prefix.substr(cur_prefix_index+1);
    } else {
      nodes[cur_node].prefix = "";
    }
    // Create the new node from the split to the next path element
    cur_node = split_node;
  }
  nodes[cur_node].value = value;
}


std::string state_impl::get_node_hash(std::string path) {
  uint32_t node_index = get_node_index(path);
  return node_index == -1 ? "" : nodes[node_index].hash;
}

//  std::vector<std::string>
//        get_node_children_as_string::string(std::string path) {
//  std::vector<std::string> result;
//  uint32_t node_index = get_node_index(path);
//  if (node_index == -1) {
//    throw std::exception("No node at this path");
//  }
//  for (unsigned int i = 0; i < 256; ++i) {
//    if (nodes[node_index].children[i]) {
//      result.push_back(std::string((const char*)&i));
//    }
//  }
//  return result;
//}

std::vector<unsigned char> state_impl::get_node_children(std::string path) {
  std::vector<unsigned char> result;
  uint32_t node_index = get_node_index(path);
  if (node_index == -1) {
    throw std::exception("No node at this path");
  }
  for (unsigned int i = 0; i < 256; ++i) {
    if (nodes[node_index].children[i]) {
      result.push_back(i);
    }
  }
  return result;
}

void state_impl::delete_node_tree(std::string path) {
}

// 1. If multiple children -> set value to ""
// 2. If one child -> merge with child and parent points to child
// 3. If no children, -> delete and remove link from parent,
//  3.1 If parent has only one child remaining, and the value of parent is "", merge parent and its remaining child
void state_impl::erase(std::string path) {
  int32_t cur_node = get_node_index(path);
  if (cur_node == -1 || nodes[cur_node].value == "") {
    throw std::exception("No set node at path at");
  }

  // Get the children of this node
  std::vector<unsigned char> children;
  for (unsigned int i = 0; i < 256; ++i) {
    if (nodes[cur_node].children[i]) {
      children.push_back(i);
    }
  }

  if (children.size() > 1) {
    nodes[cur_node].value = "";
  // if one child -> merge prefix into child and parent points to child
  } else if(children.size() == 1) {
    uint32_t parent = nodes[cur_node].parent;
    uint32_t child = nodes[cur_node].children[children[0]];
    // add the prefix of current node to the child
    nodes[child].prefix.insert(0, 1, children[0]);
    nodes[child].prefix.insert(0, nodes[cur_node].prefix);
    //  link parent to child
    unsigned char path_from_parent = path[path.length() -
        nodes[cur_node].prefix.length() - 1];
    nodes[parent].children[path_from_parent] = child;
    // TODO(Samir):  Remember empty elements for later use
    fragmented_locations.push(cur_node);
  // if no children, remove element and handle parent cases
  } else {
    uint32_t parent = nodes[cur_node].parent;
    uint32_t path_from_parent_index = path.length() -
        nodes[cur_node].prefix.length() - 1;
    unsigned char path_from_parent = path[path_from_parent_index];
    nodes[parent].children[path_from_parent] = 0;
    fragmented_locations.push(cur_node);

    //  merge parent with remaining child  if parent has no value
    children.clear();
    cur_node = parent;
    for (unsigned int i = 0; i < 256; ++i) {
      if (nodes[cur_node].children[i]) {
        children.push_back(i);
      }
    }

    if (nodes[cur_node].value.length() == 0
        && children.size() == 1 && cur_node != 0) {
      parent = nodes[cur_node].parent;
      uint32_t child = nodes[cur_node].children[children[0]];
      nodes[child].prefix.insert(0, 1, children[0]);
      nodes[child].prefix.insert(0, nodes[cur_node].prefix);

      path_from_parent_index -= nodes[cur_node].prefix.length() + 1;
      path_from_parent = path[path_from_parent_index];
      nodes[parent].children[path_from_parent] = child;
      fragmented_locations.push(cur_node);
    }
  }
}

void state_impl::commit_changes() {
}

void state_impl::discard_changes() {
}

int32_t state_impl::get_node_index(std::string path) {
  uint32_t cur_node = 0;

  for (uint32_t i = 0; i < path.length(); i++) {
    uint8_t path_element = path[i];
    // if no prefix keep looking
    if (nodes[cur_node].prefix.length() == 0) {
      if (nodes[cur_node].children[path_element] == 0) {
        return -1;
      }
      cur_node = nodes[cur_node].children[path_element];
    // else compare prefix with remaining path and decide what to do
    } else {
      // if prefix is shorter than remaining path, compare them.
      if (nodes[cur_node].prefix.length() < path.length() - i) {
        if (nodes[cur_node].prefix == path.substr(i,
            nodes[cur_node].prefix.length())) {
          i += nodes[cur_node].prefix.length();
          path_element = path[i];
          if (nodes[cur_node].children[path_element]) {
            cur_node = nodes[cur_node].children[path_element];
          } else {
            return -1;
          }
        } else {
          return -1;
        }
      // if prefix length is equal to remaining path compare
      } else if (nodes[cur_node].prefix.length() == path.length() - i) {
        if (nodes[cur_node].prefix == path.substr(i)) {
          return cur_node;
        } else {
          return -1;
        }
      // if prefix is longer than remaining path there is no node with this path
      } else {
        return -1;
      }
    }
  }
  return cur_node;
}


bool state_impl::has_children(uint32_t node_index) {
  for (unsigned int i = 0; i < 256; ++i) {
    if (nodes[node_index].children[i]) {
      return true;
    }
  }
  return false;
}

uint32_t state_impl::add_node(uint32_t from, unsigned char to) {
  nodes[from].children[to] = nodes.size();
  nodes.push_back(node());  // change to -> new node() and refactor;
  nodes[nodes[from].children[to]].parent = from;
  return nodes[from].children[to];
}
