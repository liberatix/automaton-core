#include "state/state_impl.h"
#include <algorithm>
#include <string>
#include <vector>
#include <stack>
#include "crypto/hash_transformation.h"


state_impl::state_impl(hash_transformation* hasher) {
  nodes.push_back(state_impl::node());
  this->hasher = hasher;
  calculate_hash(0);
}

std::string state_impl::get(const std::string& key) {
  int32_t node_index = get_node_index(key);
  //std::cout << "index at key\"" << key << "\": " << node_index << std::endl;
  return node_index == -1 ? "" : nodes[node_index].value;
}
void state_impl::set(const std::string& key, const std::string& value) {
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
    if (cur_prefix_index == nodes[cur_node].prefix.length()) {
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
    // If there is prefix at this node -> progress prefix
    } else {
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
        unsigned char path_to_child = cur_node_prefix[cur_prefix_index];
        nodes[split_node].children[path_to_child] = cur_node;

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
  // We are at the last key element and there is still prefix left.
  if ((nodes[cur_node].prefix.length() != cur_prefix_index)
      && i == key.length()) {
    // Create node here
    const std::string cur_node_prefix = nodes[cur_node].prefix;

    // Create the split_node and set up links with cur_node
    uint32_t split_node = add_node(nodes[cur_node].parent, key[key.length() -
        cur_prefix_index-1]);
    // Set split node as parent of cur_node
    nodes[cur_node].parent = split_node;
    // Set the current node as child of the new split node

    const unsigned char path_to_child = cur_node_prefix[cur_prefix_index];
    nodes[split_node].children[path_to_child] = cur_node;

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
  // set the value and calculate the hash
  nodes[cur_node].value = value;
  calculate_hash(cur_node);
}

// TODO(Samir): add return in hex format
std::string state_impl::get_node_hash(const std::string& path) {
  int32_t node_index = get_node_index(path);
  return node_index == -1 ? "" : nodes[node_index].hash;
}

//  std::vector<std::string>
//        get_node_children_as_string::string(const std::string& path) {
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

std::vector<unsigned char> state_impl::get_node_children(
    const std::string& path) {
  std::vector<unsigned char> result;
  int32_t node_index = get_node_index(path);
  if (node_index == -1) {
    throw std::out_of_range("No node at this path");
  }
  for (unsigned int i = 0; i < 256; ++i) {
    if (nodes[node_index].children[i]) {
      result.push_back(i);
    }
  }
  return result;
}

void state_impl::delete_node_tree(const std::string& path) {
  // TODO(Samir): Implement delete subtrie ( subtrie of node with value only? )
  throw std::exception();
}

// 1. If multiple children -> set value to ""
// 2. If one child -> merge with child and parent points to child
// 3. If no children, -> delete and remove link from parent,
//  3.1 If parent has only one child remaining, and the value of parent is "",
//      merge parent and its remaining child
void state_impl::erase(const std::string& path) {
  int32_t cur_node = get_node_index(path);
  if (cur_node == -1 || nodes[cur_node].value == "") {
    throw std::out_of_range("No set node at path at");
  }

  // Get the children of this node
  std::vector<unsigned char> children;
  for (unsigned int i = 0; i < 256; ++i) {
    if (nodes[cur_node].children[i]) {
      children.push_back(i);
    }
  }

  // If multiple children just erase the value
  if (children.size() > 1) {
    nodes[cur_node].value = "";
  // If one child -> merge prefix into child, link parent and child
  } else if (children.size() == 1) {
    uint32_t parent = nodes[cur_node].parent;
    uint32_t child = nodes[cur_node].children[children[0]];
    // add the prefix of current node to the child
    nodes[child].prefix.insert(0, 1, children[0]);
    nodes[child].prefix.insert(0, nodes[cur_node].prefix);
    // link parent and child
    unsigned char path_from_parent = path[path.length() -
        nodes[cur_node].prefix.length() - 1];
    nodes[parent].children[path_from_parent] = child;
    nodes[child].parent = parent;
    // Remember empty elements for later use
    fragmented_locations.push(cur_node);
    cur_node = child;
  // If no child -> remove element and handle parent cases
  } else {
    // erase the link from parent
    uint32_t parent = nodes[cur_node].parent;
    uint32_t path_from_parent_index = path.length() -
        nodes[cur_node].prefix.length() - 1;
    unsigned char path_from_parent = path[path_from_parent_index];
    nodes[parent].children[path_from_parent] = 0;
    fragmented_locations.push(cur_node);

    // Find out how many children does the parent have
    children.clear();
    cur_node = parent;
    for (unsigned int i = 0; i < 256; ++i) {
      if (nodes[cur_node].children[i]) {
        children.push_back(i);
      }
    }
    // If the parent of the deleted node has no prefix, has only one
    // child remaining and is not the root we will merge it with his child
    if (nodes[cur_node].value.length() == 0
        && children.size() == 1 && cur_node != 0) {
      parent = nodes[cur_node].parent;
      uint32_t child = nodes[cur_node].children[children[0]];
      nodes[child].prefix.insert(0, 1, children[0]);
      nodes[child].prefix.insert(0, nodes[cur_node].prefix);

      // link parent and child
      path_from_parent_index -= nodes[cur_node].prefix.length() + 1;
      path_from_parent = path[path_from_parent_index];
      nodes[parent].children[path_from_parent] = child;
      nodes[child].parent = parent;
      fragmented_locations.push(cur_node);
    }
  }
  calculate_hash(cur_node);
}

void state_impl::commit_changes() {
}

void state_impl::discard_changes() {
}

uint32_t state_impl::hash_size() {
  return hasher->digest_size();
}

int32_t state_impl::get_node_index(const std::string& path) {
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

void state_impl::calculate_hash(uint32_t cur_node) {
  const uint8_t *value, *prefix, *child_hash;
  hasher->restart(); // just in case

  // Hash the value
  value =
      reinterpret_cast<const uint8_t*>(nodes[cur_node].value.c_str());
  size_t len = nodes[cur_node].value.length();
  hasher->update(value, len);

  // Hash the prefix
  prefix =
      reinterpret_cast<const uint8_t*>(nodes[cur_node].prefix.c_str());
  len = nodes[cur_node].prefix.length();
  hasher->update(prefix, len);

  // Hash the children hashes
  for (int i = 0; i < 256; i++) {
    if (nodes[cur_node].children[i]) {
      uint32_t child = nodes[cur_node].children[i];
      child_hash =
          reinterpret_cast<const uint8_t*>(nodes[child].hash.c_str());
      len = nodes[child].hash.length();
      hasher->update(child_hash, len);
    }
  }
  uint8_t * digest = new uint8_t[hasher->digest_size()];
  hasher->final(digest);
  nodes[cur_node].hash = std::string(reinterpret_cast<char*>(digest),
      hasher->digest_size());

  if (cur_node != 0) {
    calculate_hash(nodes[cur_node].parent);
  }
}
