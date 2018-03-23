#ifndef AUTOMATON_CORE_STATE_IMPL_H__
#define AUTOMATON_CORE_STATE_IMPL_H__

#include "state/state.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <stack>

class state_impl : public state{
 public:
  state_impl();

  // Get the value at given path. Empty string if no value is set or
  // there is no node at the given path
  std::string get(std::string key);

  // Set the value at a given path
  void set(std::string key, std::string value);

  // Get the hash of a node at the given path. Empty std::string if no value is
  // set or there is no node at the given path
  std::string get_node_hash(std::string path);

  // Get the children as chars //TODO(Samir:) change to to return sting path to
  // children with value.
  std::vector<unsigned char> get_node_children(std::string path);

  // Erase previously set element in the trie
  void erase(std::string path);

  // delete subtree with root the node at the given path
  void delete_node_tree(std::string path);

  // finalizes the changes made by set
  void commit_changes();

  // discards the changes made by set;
  void discard_changes();

 private:
  struct node {
    uint32_t parent;
    std::string prefix;
    std::string hash;
    std::string value;
    uint32_t children[256];
  };
  std::vector<node> nodes;
  std::stack<uint32_t> fragmented_locations;

  int32_t get_node_index(std::string path);
  bool has_children(uint32_t node_index);
  uint32_t add_node(uint32_t from, unsigned char to);
};

#endif  //  AUTOMATON_CORE_STATE_IMPL_H__
