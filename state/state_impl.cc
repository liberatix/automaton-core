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
#include "state.h"
#include "state_impl.h"



state_impl::state_impl() {
  nodes.push_back(state_impl::node());
}

std::string state_impl::get(std::string key) {
  int32_t node_index = get_node_index(key);
  std::cout << "index at get for key " << key << " :" << node_index << std::endl;
  return node_index == -1 ? "" : nodes[node_index].value;
}
/*
Cases: for value len >= 1 // if at end of key 
1. prefix.size = 0, children = 0 -> place the node here with remaining path as prefix
2. prefix.size = 0, children >= 1 ->  Continue looking if children with this path exist or create new node with the this path element
3. prefix.size = >1, children = 0 -> Check if path is contained partial / fully or path is longer than the prefix
1. remaining parth is equal in lenght and matches the prefix -> set the value
1. Else split
2. remaining path is longer than prefix
1. matches full prefix -> continue looking
2. matches partial prefix -> split the node at the difference
3. remaining path is shorter than prefix
1. full remainig path is contained in prefix -> create node at the end of the path and make the current one its child ( with shortend prefix )
2. remaining path is partialy matches the prefix -> split the node at missmatch and make both nodes chilldren to it 
( previous node get its prefix shortend and new one get position under the split with remaining path as prefix )
4. prefix.size = >1, children > = 1 -> 
1. remaining path is equal in lenght and matches the prefix -> set the value
1. Else split
2. remaining path is longer than prefix
1. matches full prefix -> Continue looking if children with this path exist or create new node with the this path element
2. matches partial prefix -> split the node at the difference
3. remaining path is shorter than prefix
1. full remainig path is contained in prefix -> create node at the end of the path and make the current one its child ( with shortend prefix )
2. remaining path is partialy matches the prefix -> split the node at missmatch and make both nodes chilldren to it 
( previous node get its prefix shortend and new one get position under the split with remaining path as prefix )
Cases: for value len == 0 ( delete this node )
*/
void state_impl::set(std::string key, std::string value) {  // value = "" means erase()
                                              // If value == "" erase()
                                              // else set();
  if (value == "") {
    erase(key);
    return;
  }
  uint32_t cur_node = 0;
  uint32_t cur_prefix_index = 0;
  unsigned int i = 0;
  for (; i < key.length(); ++i) {
    unsigned char path_element = key[i];
    // If there is no prefix
    // Continue looking 
    if (nodes[cur_node].prefix.length() == 0 || 
      (cur_prefix_index != 0 && cur_prefix_index == nodes[cur_node].prefix.length())) {

      if (nodes[cur_node].children[path_element] != 0) {
        // If there is children with the next path element continue on the path
        cur_node = nodes[cur_node].children[path_element];
      }
      else if (has_children(cur_node) || nodes[cur_node].value != "" || cur_node == 0) {
        // This node has children, value set or is the root, we can't change its prefix so we continue
        // add_node(from, to);
        cur_node = add_node(cur_node, path_element); // -> this line replaces the next 4
                                                      //nodes[cur_node].children[path_element] = nodes.size();
                                                      //nodes.push_back(node()); // change to -> new node() and refactor;
                                                      //nodes[cur_node].children[path_element].parent = cur_node; -> wrong line
                                                      //cur_node = nodes[cur_node].children[path_element];
      }
      // if the remaining key is more than a single char 
      else { 
        // this node has no children so it's the final node and the remainder of the path will be the prefix
        nodes[cur_node].prefix = key.substr(i);
        //cur_prefix_index = nodes[cur_node].prefix.length();
        break;
      }
      cur_prefix_index = 0;
    }
    // If there is prefix at this node -> progress prefix
    // TODO(Samir): Consider: what if path ends before the prefix? how do we handle it?
    else { 
      // If next path element does not match the next prefix element,
      // create a split node at the difference
      if (path_element != nodes[cur_node].prefix[cur_prefix_index]) {
        const std::string cur_node_prefix = nodes[cur_node].prefix;

        // Create the split_node and set up links with cur_node
        uint32_t split_node = add_node(nodes[cur_node].parent, key[i-cur_prefix_index-1]);
        // Set split node as parent of cur_node
        nodes[cur_node].parent = split_node;
        // Set the current node as child of the new split node
        nodes[split_node].children[cur_node_prefix[cur_prefix_index]] = cur_node;

        // set prefix of split_node and cur_node
        if (cur_prefix_index >= 1) {
          nodes[split_node].prefix = cur_node_prefix.substr(0, cur_prefix_index);
        }
        if(cur_node_prefix.length() - cur_prefix_index > 1) {
          nodes[cur_node].prefix = cur_node_prefix.substr(cur_prefix_index+1);
        }
        else {
          nodes[cur_node].prefix = "";
        }
        // Create the new node from the split to the next path element
        cur_node = add_node(split_node, path_element);
        cur_prefix_index = 0;
      }
      // TODO(Samir): else if we are at the last path_element ? -> handle the split here?
      //else if (i+1 == key.length() && cur_prefix_index+1 != nodes[cur_node].prefix.length()) {
      //  // We are at the last key element and there is still prefix left.
      //  // Create node here 
      //  const std::string cur_node_prefix = nodes[cur_node].prefix;
      //
      //  // Create the split_node and set up links with cur_node
      //  uint32_t split_node = add_node(nodes[cur_node].parent, key[i-cur_prefix_index-1]);
      //  // Set split node as parent of cur_node
      //  nodes[cur_node].parent = split_node;
      //  // Set the current node as child of the new split node
      //  nodes[split_node].children[cur_node_prefix[cur_prefix_index+1]] = cur_node;
      //
      //  // set prefix of split_node and cur_node
      //  if (cur_prefix_index >= 1) {
      //    nodes[split_node].prefix = cur_node_prefix.substr(0, cur_prefix_index+1);
      //  }
      //  if(cur_node_prefix.length() - cur_prefix_index > 1) {
      //    nodes[cur_node].prefix = cur_node_prefix.substr(cur_prefix_index+2);
      //  }
      //  else {
      //    nodes[cur_node].prefix = "";
      //  }
      //  // Create the new node from the split to the next path element
      //  cur_node = split_node;
      //}
      else {
        ++cur_prefix_index;
      }
    }
  }
  if ((nodes[cur_node].prefix.length() != cur_prefix_index) && i == key.length()) {
    // We are at the last key element and there is still prefix left.
    // Create node here 
    const std::string cur_node_prefix = nodes[cur_node].prefix;

    // Create the split_node and set up links with cur_node
    uint32_t split_node = add_node(nodes[cur_node].parent, key[key.length()-cur_prefix_index-1]);
    // Set split node as parent of cur_node
    nodes[cur_node].parent = split_node;
    // Set the current node as child of the new split node
    nodes[split_node].children[cur_node_prefix[cur_prefix_index]] = cur_node;

    // set prefix of split_node and cur_node
    if (cur_prefix_index >= 1) {
      nodes[split_node].prefix = cur_node_prefix.substr(0, cur_prefix_index);
    }
    if(cur_node_prefix.length() - cur_prefix_index > 1) {
      nodes[cur_node].prefix = cur_node_prefix.substr(cur_prefix_index+1);
    }
    else {
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

//virtual std::vector<std::string> get_node_children_as_std::string(std::string path) {
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
  // get_node

}
// 1. If multiple children -> set value to ""
// 2. If one child -> merge with child and parent points to child
// 3. If no children, -> delete and remove link from parent,
//  3.1 If parent has only one child remaining, and the value of parent is "", merge parent and its remaining child
void state_impl::erase(std::string path) {
  // TODO: handle memory fragmentation? 
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
  }
  // if one child -> merge prefix into child and parent points to child
  else if(children.size() == 1) { 
    uint32_t parent = nodes[cur_node].parent;
    uint32_t child = nodes[cur_node].children[children[0]];
    // add the prefix of current node to the child
    nodes[child].prefix.insert(0, 1, children[0]);
    nodes[child].prefix.insert(0, nodes[cur_node].prefix);
    // link parent to child 
    unsigned char path_from_parent = path[path.length() - nodes[cur_node].prefix.length() - 1];
    nodes[parent].children[path_from_parent] = child;
    // TODO(Samir):  Remember empty elements for later use
    fragmented_locations.push(cur_node);
  }
  // if no children, remove element and handle parent cases
  else {
    // TODO(Samir) Implement:  merge_with_child_if_necesery(parent)
    uint32_t parent = nodes[cur_node].parent;
    uint32_t path_from_parent_index = path.length() - nodes[cur_node].prefix.length() - 1;
    unsigned char path_from_parent = path[path_from_parent_index];
    nodes[parent].children[path_from_parent] = 0;
    fragmented_locations.push(cur_node);


    //merge_ancestors_while_necesery(parent);
    children.clear();
    cur_node = parent;
    for (unsigned int i = 0; i < 256; ++i) {
      if (nodes[cur_node].children[i]) {
        children.push_back(i);
      }
    }

    if (nodes[cur_node].value.length() == 0 && children.size() == 1 && cur_node != 0) {
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
  // 
  uint32_t cur_node = 0;

  for (uint32_t i = 0; i < path.length(); i++) {
    uint8_t path_element = path[i];
    // if no prefix keep looking
    if (nodes[cur_node].prefix.length() == 0) {
      if (nodes[cur_node].children[path_element] == 0) {
        return -1;
      }
      cur_node = nodes[cur_node].children[path_element];
    }
    // else compare prefix with remaining path and decide what to do
    else {
      // if prefix is shorter than remaining path, compare them. 
      if (nodes[cur_node].prefix.length() < path.length() - i) {
        if (nodes[cur_node].prefix == path.substr(i, nodes[cur_node].prefix.length())) {
          i += nodes[cur_node].prefix.length();
          path_element = path[i];
          if (nodes[cur_node].children[path_element]) {
            cur_node = nodes[cur_node].children[path_element];
          }
          else {
            return -1;
          }
        }
        else {
          return -1;
        }
      }
      // if prefix length is equal to remaining path compare and return index or -1
      else if (nodes[cur_node].prefix.length() == path.length() - i) {
        if (nodes[cur_node].prefix == path.substr(i)) {
          return cur_node;
        }
        else {
          return -1;
        }
      }
      // if prefix is longer than remaining path there is no node with this path
      else {
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
  nodes.push_back(node()); // change to -> new node() and refactor;
  nodes[nodes[from].children[to]].parent = from;
  return nodes[from].children[to];
}
