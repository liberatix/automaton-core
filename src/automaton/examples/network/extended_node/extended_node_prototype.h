#ifndef AUTOMATON_EXAMPLES_NETWORK_EXTENDED_NODE_EXTENDED_NODE_PROTOTYPE_H_
#define AUTOMATON_EXAMPLES_NETWORK_EXTENDED_NODE_EXTENDED_NODE_PROTOTYPE_H_

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "automaton/core/crypto/hash_transformation.h"
#include "automaton/core/data/schema.h"
#include "automaton/core/data/factory.h"
#include "automaton/core/data/msg.h"
#include "automaton/core/data/protobuf/protobuf_msg.h"
#include "automaton/core/state/state.h"
#include "automaton/core/network/connection.h"
#include "automaton/core/network/acceptor.h"
#include "automaton/examples/crypto/basic_hash_miner.h"

namespace automaton {
namespace examples {

/// Class node
class node {
 public:
  // TODO(kari): This need to be deleted.
  static core::data::factory* msg_factory;
  struct block {
    std::string hash;
    std::string prev_hash;
    uint32_t height;
    std::string miner;

    std::string to_string() const;

    block();

    block(std::string hash, std::string prev_hash, uint32_t height, std::string miner);
  };
  class handler: public core::network::connection::connection_handler {
   public:
    node* node_;

    explicit handler(node* n);

    void on_message_received(core::network::connection* c, char* buffer,
        uint32_t bytes_read, uint32_t id);

    void on_message_sent(core::network::connection* c, uint32_t id,
        core::network::connection::error e);

    void on_connected(core::network::connection* c);

    void on_disconnected(core::network::connection* c);

    void on_error(core::network::connection* c,
        core::network::connection::error e);
  };
  class lis_handler: public core::network::acceptor::acceptor_handler {
   public:
    node* node_;

    explicit lis_handler(node* n);

    bool on_requested(const std::string& address);

    void on_connected(core::network::connection* c, const std::string& address);

    void on_error(core::network::connection::error e);
  };
  node();

  ~node();

  bool init();

  uint32_t id;

  void mine(const std::string& new_hash);

  char* add_buffer(uint32_t size);

  /// This function is created because the acceptor needs ids for the connections it accepts
  uint32_t get_next_peer_id();

  bool accept_connection();

  bool add_peer(uint32_t id, const std::string& connection_type, const std::string& address);

  void remove_peer(uint32_t id);

  bool add_acceptor(uint32_t id, const std::string& connection_type, const std::string& address);

  void remove_acceptor(uint32_t id);

  void send_message(const std::string& message, uint32_t connection_id = 0);

  void handle_block(const std::string& hash, const block& block_,
      const std::string& serialized_block);

  std::pair<uint32_t, std::string> get_height_and_top();

  // process

 private:
  std::string chain_top;
  uint32_t height;
  bool initialized;
  // TODO(kari): remove this and add hello message passing id as well as a list of ids and peers
  uint32_t peer_ids;  // count
  handler* handler_;
  lis_handler* lis_handler_;
  std::vector<char*> buffers;
  std::mutex buffer_mutex;
  std::mutex global_state_mutex;
  std::mutex orphan_blocks_mutex;
  std::mutex peer_ids_mutex;
  std::mutex peers_mutex;
  std::mutex acceptors_mutex;
  std::mutex chain_top_mutex;
  std::mutex height_mutex;
  std::map<std::string, block> orphan_blocks;
  std::map<uint32_t, core::network::acceptor*> acceptors;
  std::map<uint32_t, core::network::connection*> peers;
  core::crypto::hash_transformation* hasher;
  core::state::state* global_state;  // map block_hash -> serialized msg, containing the block
  // basic_hash_miner* miner

  void check_orphans();

  std::string create_send_blocks_message(std::vector<std::string> hashes);

  std::string create_request_blocks_message(std::vector<std::string> hashes);

  /// Helper functions

  block msg_to_block(core::data::msg* message) const;

  std::unique_ptr<core::data::msg> block_to_msg(const block& block) const;

  std::string hash_block(const block& block) const;
};

}  // namespace examples
}  // namespace automaton

#endif  // AUTOMATON_EXAMPLES_NETWORK_EXTENDED_NODE_EXTENDED_NODE_PROTOTYPE_H_
