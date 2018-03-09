#ifndef AUTOMATON_CORE_CRYPTO_STATE_TRANSFORMATION_H__
#define AUTOMATON_CORE_CRYPTO_STATE_TRANSFORMATION_H__

#include <string>
#include <vector>

class state {
 public:
  // Get the value at given path. Empty string if no value is set or
  // there is no node at the given path
  virtual std::string get(std::string key) = 0;

  // Set the value at a given path
  virtual void set(std::string key, std::string value) = 0;

  // Get the hash of a node at the given path. Empty std::string if no value is
  // set or there is no node at the given path
  virtual std::string get_node_hash(std::string path) = 0;

  // Get the children as std::string
  virtual std::vector<std::string> get_node_children(std::string path) = 0;

  // delete subtree with root the node at the given path
  virtual void delete_node_tree(std::string path) = 0;

  // finalizes the changes made by set
  virtual void commit_changes() = 0;

  // discards the changes made by set;
  virtual void discard_changes() = 0;
};
#endif  //  AUTOMATON_CORE_CRYPTO_STATE_TRANSFORMATION_H__
