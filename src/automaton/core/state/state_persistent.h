#ifndef AUTOMATON_CORE_STATE_STATE_PERSISTENT_H__
#define AUTOMATON_CORE_STATE_STATE_PERSISTENT_H__

#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include <set>
#include "automaton/core/state/state.h"
#include "automaton/core/crypto/hash_transformation.h"
#include "automaton/core/storage/blobstore.h"

namespace automaton {
namespace core {
namespace state {

class state_persistent : public state{
 public:
  explicit state_persistent(crypto::hash_transformation* hasher, storage::blobstore* bs);

  // Get the value at given path. Empty string if no value is set or
  // there is no node at the given path
  std::string get(const std::string& key);

  // Set the value at a given path
  void set(const std::string& key, const std::string& value);

  // Get the hash of a node at the given path. Empty std::string if no value is
  // set or there is no node at the given path
  std::string get_node_hash(const std::string& path);

  // Get the children as chars //TODO(Samir:) change to to return sting path to
  // children with value.
  std::vector<std::string> get_node_children(const std::string& path);

  // Erase previously set element in the trie
  void erase(const std::string& path);

  // delete subtree with root the node at the given path
  void delete_node_tree(const std::string& path);

  // finalizes the changes made by set
  void commit_changes();

  // discards the changes made by set;
  void discard_changes();

  // get the size of the hash in bytes
  uint32_t hash_size();

  // compare differences in with the second state and print path
  void print_subtrie(std::string path, std::string formated_path);

  uint32_t size();

 private:
  class node {
   public:
     uint32_t get_parent(storage::blobstore* bs) {
       uint32_t sz;
       uint8_t* p_parent = bs->get(parent_, &sz);
       return *(reinterpret_cast<uint32_t*>(p_parent));
       // return parent_;
     }
     std::string get_prefix(storage::blobstore* bs) {
       uint32_t sz;
       uint8_t* p_prefix = bs->get(prefix_, &sz);
       return std::string(reinterpret_cast<char*>(p_prefix), sz);
       // return prefix_;
     }
     std::string get_hash(storage::blobstore* bs) {
       uint32_t sz;
       uint8_t* p_hash = bs->get(hash_, &sz);
       return std::string(reinterpret_cast<char*>(p_hash), sz);
       // return hash_;
     }
     std::string get_value(storage::blobstore* bs) {
       uint32_t sz;
       uint8_t* p_value = bs->get(value_, &sz);
       return std::string(reinterpret_cast<char*>(p_value), sz);
       // return value_;
     }
     uint32_t get_child(uint8_t child, storage::blobstore* bs) {
       return children_[child];
     }

     void set_parent(uint32_t parent, storage::blobstore* bs) {
       parent_ = bs->store(sizeof(uint32_t), reinterpret_cast<uint8_t*>(&parent));
       // parent_ = parent;
     }
     void  set_prefix(const std::string prefix, storage::blobstore* bs) {
       prefix_ = bs->store(prefix.length(),
           reinterpret_cast<const uint8_t*>(prefix.c_str()));
       // prefix_ = prefix;
     }
     void  set_hash(const std::string hash, storage::blobstore* bs) {
       hash_ = bs->store(hash.length(),
         reinterpret_cast<const uint8_t*>(hash.c_str()));
       // hash_ = hash;
     }
     void set_value(const std::string value, storage::blobstore* bs) {
       value_ = bs->store(value.length(),
       reinterpret_cast<const uint8_t*>(value.c_str()));
       // value_ = value;
     }
     void set_child(const uint8_t child, const uint32_t value, storage::blobstore* bs) {
       children_[child] = value;
     }

   public:
    uint32_t parent_ = 0;
    // std::string prefix_;
    // std::string hash_;
    // std::string value_;
    uint64_t prefix_ = 0;
    uint64_t hash_ = 0;
    uint64_t value_ = 0;
    uint32_t children_[256] = {};
  };
  storage::blobstore* bs;

  std::vector<node> nodes;
  std::map<uint32_t, node> backup;
  std::set<uint32_t> free_locations;
  crypto::hash_transformation* hasher;
  uint32_t nodes_current_state;
  uint32_t permanent_nodes_count;

  int32_t get_node_index(const std::string& path);
  bool has_children(uint32_t node_index);
  uint32_t add_node(uint32_t from, unsigned char to);
  // This needs to be called at the end of set() and erase() to recalculate the
  // hashes of all nodes from lowest child that was changed to the root
  void calculate_hash(uint32_t cur_node);
  // Create a backup starting from cur_node to root if they are not
  // in the backup map
  void backup_nodes(uint32_t cur_node);
  // add all nodes in the subtrie to free_locations to be reused.
  void subtrie_mark_free(uint32_t cur_node);
};

}  // namespace state
}  // namespace core
}  // namespace automaton

#endif  //  AUTOMATON_CORE_STATE_STATE_PERSISTENT_H__
