#include "automaton/examples/network/extended_node/extended_node_prototype.h"

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include "automaton/core/data/schema.h"
#include "automaton/core/data/factory.h"
#include "automaton/core/data/msg.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "automaton/core/data/protobuf/protobuf_msg.h"
#include "automaton/core/crypto/hash_transformation.h"
#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/io/io.h"
#include "automaton/core/log/log.h"
#include "automaton/core/state/state_impl.h"


using automaton::core::data::msg;
using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::data::protobuf::protobuf_msg;
using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::crypto::SHA256_cryptopp;
using automaton::core::crypto::hash_transformation;
using automaton::core::network::acceptor;
using automaton::core::network::connection;
using automaton::core::state::state_impl;

namespace automaton {
namespace examples {

static const char* PROTO_FILE = "automaton/examples/network/extended_node/sp.proto";
static const uint32_t HASH_SIZE = 32;

core::data::factory* node::msg_factory = nullptr;

/// Node's connection handler

node::handler::handler(node* n): node_(n) {}

void node::handler::on_message_received(connection* c, char* buffer, uint32_t bytes_read,
    uint32_t id) {
  try {
    std::string data = std::string(buffer, bytes_read);
    if (data == "") {
      c->async_read(buffer, 256, 0, 0);
      return;
    }
    // LOG(DEBUG) << id << " receiving msg: 0";
    std::unique_ptr<msg> received_msg = msg_factory->new_message_by_name("data");
    received_msg->deserialize_message(data);
    std::unique_ptr<msg> msg_type = received_msg->get_message(2);  // response
    if (msg_type) {
      // // LOG(DEBUG) << id << " receiving msg: 1";
      uint32_t hashes_number = msg_type->get_repeated_field_size(1);
      for (uint32_t i = 0; i < hashes_number; ++i) {
        // // LOG(DEBUG) << id << " receiving msg: loop " << i;
        std::string hash = msg_type->get_repeated_blob(1, i);
        node_->global_state_mutex.lock();
        node_->orphan_blocks_mutex.lock();
        if (node_->global_state->get(hash) == "" &&
            node_->orphan_blocks.find(hash) == node_->orphan_blocks.end()) {
          // // LOG(DEBUG) << id << " receiving msg: loop " << i << "if";
          node_->orphan_blocks_mutex.unlock();
          node_->global_state_mutex.unlock();
          std::string serialized_block = msg_type->get_repeated_blob(2, i);
          std::unique_ptr<msg> block_msg = msg_factory->new_message_by_name("block");
          block_msg->deserialize_message(serialized_block);
          block b = node_->msg_to_block(block_msg.get());
          // LOG(DEBUG) << "Received block: " << b.to_string();
          /// validate block
          if (!hash.compare(node_->hash_block(b))) {
            // LOG(DEBUG) << "Received valid block! Block send to handler!";
            node_->handle_block(hash, b, serialized_block);
          } else {
            LOG(ERROR) << "Block validation failed!";
          }
        } else {
          // // LOG(DEBUG) << id << " receiving msg: loop " << i << "else";
          node_->orphan_blocks_mutex.unlock();
          node_->global_state_mutex.unlock();
        }
      }
    } else {
      // LOG(DEBUG) << id << " receiving msg: 2";
      msg_type = received_msg->get_message(1);  // request
      if (!msg_type) {
        std::stringstream msg;
        msg << "Received message is empty! data msg is " <<
            core::io::string_to_hex(received_msg->to_string());
        LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
        throw std::runtime_error(msg.str());
      }
      std::vector<std::string> hashes;
      uint32_t hashes_number = msg_type->get_repeated_field_size(2);
      if (hashes_number > 0) {
        for (uint32_t i = 0; i < hashes_number; i++) {
          hashes.push_back(msg_type->get_repeated_blob(2, i));
        }
      }
      // // LOG(DEBUG) << id << " receiving msg: 3";
      std::string top_block_hash = msg_type->get_blob(1);
      // // LOG(DEBUG) << "TOP :: " << top_block_hash;
      if (top_block_hash != node_->get_top()) {
        hashes.push_back(node_->get_top());
      }
      if (hashes.size() > 0) {
      //  // LOG(DEBUG) << "SENDING :: " << hashes;
        c->async_send(node_->create_send_blocks_message(hashes));
      }
      // // LOG(DEBUG) << "sending top 1";
      // c->async_send(node_->create_send_blocks_message({node_->get_top()}));
      // // LOG(DEBUG) << "sending top 2";
    }
    c->async_read(buffer, 256, 0, 0);
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << e.what();
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
  } catch (...) {
    LOG(ERROR) << el::base::debug::StackTrace();
  }
  // LOG(DEBUG) << id << " receiving msg: end";
}

void node::handler::on_message_sent(connection* c, uint32_t id, connection::error e) {
  if (e) {
    LOG(ERROR) << "Message with id " << std::to_string(id) << " was NOT sent to " <<
        c->get_address() << "\nError " << std::to_string(e);
  } else {
    // logging("Message with id " + std::to_string(id) + " was successfully sent to " +
    //    c->get_address());
  }
}

void node::handler::on_connected(connection* c) {
  // LOG(INFO) << node_->id << " connected with: " + c->get_address();
  c->async_send(node_->create_request_blocks_message({}));
  c->async_read(node_->add_buffer(256), 256, 0, 0);
}

void node::handler::on_disconnected(connection* c) {
  // LOG(INFO) << node_->id << " disconnected with: " + c->get_address();
}

void node::handler::on_error(connection* c, connection::error e) {
  if (e == connection::no_error) {
    return;
  }
  LOG(ERROR) << "Error: " << std::to_string(e) << " (connection " << c->get_address() << ")";
}

/// Node's acceptor handler

node::lis_handler::lis_handler(node* n):node_(n) {}

bool node::lis_handler::on_requested(const std::string& address) {
  // EXPECT_EQ(address, address_a);
  // LOG(INFO) << node_->id << " received connection request from : " + address;
  return node_->accept_connection(/*address*/);
}

void node::lis_handler::on_connected(connection* c, const std::string& address) {
  // LOG(INFO) << node_->id << " accepted connection from " << address;
  node_->add_peer(c, address);
  c->async_read(node_->add_buffer(256), 256, 0, 0);
}

void node::lis_handler::on_error(connection::error e) {
  LOG(ERROR) << std::to_string(e);
}

/// Node

node::node():id(""), first_block_hash(std::string(HASH_SIZE, 0)), chain_top(first_block_hash),
    height(0), initialized(false), peer_ids(0) {}

bool node::init() {
  if (initialized) {
    return false;
  }
  try {
    if (!msg_factory) {
      msg_factory = new protobuf_factory();
      protobuf_schema loaded_schema(core::io::get_file_contents(PROTO_FILE));
      // LOG(DEBUG) << "SCHEMA::" << loaded_schema.dump_schema();
      msg_factory->import_schema(&loaded_schema, "proto", "");
      SHA256_cryptopp::register_self();
    }
    handler_ = new handler(this);
    lis_handler_ = new lis_handler(this);
    nonce = new uint8_t[HASH_SIZE]();
    hasher = hash_transformation::create("SHA256");
    global_state = new state_impl(hasher);
    return initialized = true;
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << e.what();
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
    return false;
  } catch (...) {
    LOG(ERROR) << el::base::debug::StackTrace();
    return false;
  }
}

node::~node() {
  for (uint32_t i = 0; i < buffers.size(); ++i) {
    delete [] buffers[i];
  }
  // delete global_state;
  // delete hasher;
  for (auto it = peers.begin(); it != peers.end(); ++it) {
    delete it->second;
  }
  for (auto it = acceptors.begin(); it != acceptors.end(); ++it) {
    delete it->second;
  }
}

node::block::block() {}

node::block::block(std::string hash, std::string prev_hash, uint32_t height,
    std::string miner, std::string nonce):
    hash(hash), prev_hash(prev_hash), height(height), miner(miner), nonce(nonce) {}

std::string node::block::to_string() const {
  std::stringstream stream;
  stream << "block {\n\thash :" << automaton::core::io::string_to_hex(hash) << "\n\tprev_hash: " <<
      automaton::core::io::string_to_hex(prev_hash) << "\n\theight: " << height << "\n\tminer: " <<
      miner << "\n\tnonce: " << automaton::core::io::string_to_hex(nonce) << "\n}";
  return stream.str();
}

char* node::add_buffer(uint32_t size) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(buffer_mutex);
  buffers.push_back(new char[size]);
  return buffers[buffers.size() - 1];
}

/// This function is created because the acceptor needs ids for the connections it accepts
uint32_t node::get_next_peer_id() {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peer_ids_mutex);
  return ++peer_ids;
}

bool node::accept_connection() {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  return true;
}

bool node::add_peer(const std::string& id, const std::string& connection_type,
    const std::string& address) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = peers.find(id);
  if (it != peers.end()) {
    delete it->second;
  }
  connection* new_connection;
  try {
    new_connection = connection::create(connection_type, address, handler_);
  } catch (std::exception& e) {
    LOG(ERROR) << e.what();
    peers[id] = nullptr;
    return false;
  }
  if (!new_connection) {
    LOG(ERROR) << "Connection was not created!";  // Possible reason: tcp_init was never called
    return false;
  }
  peers[id] = new_connection;
  new_connection->connect();
  return true;
}

bool node::add_peer(automaton::core::network::connection* c, const std::string& address) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = peers.find(address);
  if (it != peers.end() && c != it->second) {
    // LOG(DEBUG) << "Peer with this id already exists and will be replaced!";
    delete it->second;
  }
  peers[address] = c;
  return true;
}

void node::remove_peer(const std::string& id) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = peers.find(id);
  if (it != peers.end()) {
    peers.erase(it);
  }
}

bool node::add_acceptor(const std::string& id, const std::string& connection_type,
    const std::string& address) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(acceptors_mutex);
  auto it = acceptors.find(id);
  if (it != acceptors.end()) {
    // LOG(DEBUG) << "Acceptor with this id already exists!";
    delete it->second;
  }
  acceptor* new_acceptor;
  try {
    new_acceptor = acceptor::create(connection_type, address, lis_handler_, handler_);
  } catch (std::exception& e) {
    LOG(ERROR) << e.what();
    acceptors[id] = nullptr;
    return false;
  }
  if (!new_acceptor) {
    LOG(ERROR) << "Acceptor was not created!";
    return false;
  }
  acceptors[id] = new_acceptor;
  new_acceptor->start_accepting();
  return true;
}

void node::remove_acceptor(const std::string& id) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";

  std::lock_guard<std::mutex> lock(acceptors_mutex);
  auto it = acceptors.find(id);
  if (it != acceptors.end()) {
    acceptors.erase(it);
  }
}

void node::send_message(const std::string& message, const std::string& connection_id) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peers_mutex);
  if (connection_id == "") {
    for (auto it = peers.begin(); it != peers.end(); ++it) {
      // TODO(kari): if connected
      it->second->async_send(message, 0);
    }
  } else {
    peers[connection_id]->async_send(message, 0);
  }
}

void node::handle_block(const std::string& hash, const block& block_,
    const std::string& serialized_block) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  // LOG(DEBUG) << id << " handling block " << core::io::string_to_hex(hash) << '\n' <<
      block_.to_string();
  global_state_mutex.lock();
  orphan_blocks_mutex.lock();
  height_mutex.lock();
  chain_top_mutex.lock();

  std::string serialized_prev_block = global_state->get(block_.prev_hash);
  /// If we don't have the previous block
  if (block_.prev_hash == first_block_hash && block_.height != 1) {
    // LOG(DEBUG) << "HANDLE 1";
    chain_top_mutex.unlock();
    height_mutex.unlock();
    orphan_blocks_mutex.unlock();
    global_state_mutex.unlock();
    LOG(ERROR) << "Invalid block height!";
    return;
  } else if (block_.prev_hash != first_block_hash && serialized_prev_block == "") {
    // LOG(DEBUG) << "HANDLE 0";
    chain_top_mutex.unlock();
    orphan_blocks[hash] = block_;
    if (orphan_blocks.find(block_.prev_hash) == orphan_blocks.end()) {
      send_message(create_request_blocks_message({block_.prev_hash}));
    }
    height_mutex.unlock();
    orphan_blocks_mutex.unlock();
    global_state_mutex.unlock();
    return;
  /// If this is the second block
  } else if (serialized_prev_block != "") {
    // LOG(DEBUG) << "HANDLE 2";
    std::unique_ptr<msg> deserialized_prev_block = msg_factory->new_message_by_name("block");
    deserialized_prev_block->deserialize_message(serialized_prev_block);
    // If height is not what is expected, throw the block
    if (deserialized_prev_block->get_uint32(3) + 1 != block_.height) {
      chain_top_mutex.unlock();
      height_mutex.unlock();
      orphan_blocks_mutex.unlock();
      global_state_mutex.unlock();
      LOG(ERROR) << "Invalid block height!";
      return;
    }
  }
  // LOG(DEBUG) << id << " ADDING BLOCK: " << core::io::string_to_hex(hash);
  global_state->set(hash, serialized_block);
  std::string old_chain_top = chain_top;
  /// If the new block is extending the main chain
  if (block_.prev_hash == chain_top || block_.height > height) {
    chain_top = hash;
    height = block_.height;
  }
  // LOG(DEBUG) << id << " checking orphans";
  check_orphans();
  // LOG(DEBUG) << id << " end of checking orphans";
  std::string top = chain_top;
  chain_top_mutex.unlock();
  height_mutex.unlock();
  orphan_blocks_mutex.unlock();
  global_state_mutex.unlock();
  // LOG(DEBUG) << id << " unlocked";
  if (old_chain_top != top) {
    send_message(create_send_blocks_message({top}));
  }
  // LOG(DEBUG) << id << " end of handle";
}

std::pair<uint32_t, std::string> node::get_height_and_top() {
  // LOG(DEBUG) << "node " << id << " ::get 0";
  std::lock_guard<std::mutex> height_lock(height_mutex);
    // LOG(DEBUG) << "node " << id << " ::get 0.5";
  std::lock_guard<std::mutex> top_lock(chain_top_mutex);
  // LOG(DEBUG) << "node " << id << " ::get 1";
  // ==== FOR DEBUG ==== {
  // std::lock_guard<std::mutex> state_lock(global_state_mutex);
  // std::string serialized_block = global_state->get(chain_top);
  // std::unique_ptr<msg> deserialized_block = msg_factory->new_message_by_name("block");
  // deserialized_block->deserialize_message(serialized_block);
  // if (deserialized_block->get_uint32(3) != height) {
  //   LOG(ERROR) << "ERROR:: TOP BLOCK HEIGHT DOESN'T MATCH NODE HEIGHT!!!";
  // }
  // ===================
  return std::make_pair(height, chain_top);
}

std::string node::get_top() {
  std::lock_guard<std::mutex> lock(chain_top_mutex);
  return chain_top;
}

uint32_t node::get_height() {
  std::lock_guard<std::mutex> lock(height_mutex);
  return height;
}

// Private functions
// void node::check_orphans() {
//   bool erased;
//   // LOG(DEBUG) << id << " checking start " << "top: " << core::io::string_to_hex(chain_top) <<
//       " h: " << height;
//   do {
//     erased = false;
//     for (auto it = orphan_blocks.begin(); it != orphan_blocks.end(); ++it) {
//       // LOG(DEBUG) << id << " here 2 HASH: " << core::io::string_to_hex(it->first);
//       block b = it->second;
//       std::string hash = it->first;
//       std::string serialized_prev_block = global_state->get(b.prev_hash);
//        // LOG(DEBUG) << "here 3";
//       /// If the previous block is in the state
//       if (serialized_prev_block != "") {
//         // LOG(DEBUG) << "here 4";
//         std::unique_ptr<msg> deserialized_prev_block = msg_factory->new_message_by_name("block");
//         deserialized_prev_block->deserialize_message(serialized_prev_block);
//         // If height is not what is expected, throw the block
//         if (deserialized_prev_block->get_uint32(3) + 1 != b.height) {
//           LOG(ERROR) << "Invalid block height!";
//           orphan_blocks.erase(it);
//           erased = true;
//           break;
//         }
//          // LOG(DEBUG) << "here 5";
//         std::unique_ptr<msg> block_msg = block_to_msg(b);
//         if (!block_msg) {
//           std::stringstream msg;
//           msg << "Message is null";
//           LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
//           throw std::runtime_error(msg.str());
//         }
//         std::string serialized_block;
//         block_msg->serialize_message(&serialized_block);
//         // LOG(DEBUG) << id << " ADDING BLOCK: " << automaton::core::io::string_to_hex(hash);
//         global_state->set(hash, serialized_block);
//         orphan_blocks.erase(it);
//         // LOG(DEBUG) << "here 6";
//         erased = true;
//         if (b.prev_hash == chain_top || b.height > height) {
//           // LOG(DEBUG) << "here 7.0";
//           chain_top = hash;
//           height = b.height;
//           // LOG(DEBUG) << "here 7.1";
//         }
//         // LOG(DEBUG) << "here 8";
//         break;
//       } else if (it == (orphan_blocks.end()--)) {
//         // LOG(DEBUG) << "here 10";
//         erased = false;
//       }
//     }
//   } while (erased && orphan_blocks.size() > 0);
//   // LOG(DEBUG) << "checking end";
// }

void node::check_orphans() {
  bool erased;
  do {
    erased = false;
    for (auto it = orphan_blocks.begin(); it != orphan_blocks.end() && !erased; ++it) {
      std::string hash = it->first;
      block b = it->second;
      std::string serialized_prev_block = global_state->get(b.prev_hash);
      if (serialized_prev_block != "") {
        std::unique_ptr<msg> deserialized_prev_block = msg_factory->new_message_by_name("block");
        deserialized_prev_block->deserialize_message(serialized_prev_block);
        if (deserialized_prev_block->get_uint32(3) + 1 != b.height) {
          orphan_blocks.erase(it);
          erased = true;
          break;
        }
        std::unique_ptr<msg> block_msg = block_to_msg(b);
        if (!block_msg) {
          std::stringstream msg;
          msg << "Message is null";
          LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
          throw std::runtime_error(msg.str());
        }
        std::string serialized_block;
        block_msg->serialize_message(&serialized_block);
        global_state->set(hash, serialized_block);
        orphan_blocks.erase(it);
        erased = true;
        if (b.prev_hash == chain_top || b.height > height) {
          chain_top = hash;
          height = b.height;
        }
      }
    }
  } while (erased && orphan_blocks.size() > 0);
}

std::string node::create_send_blocks_message(std::vector<std::string> hashes) {
  // LOG(DEBUG) << id << " create send blocks msg";
  try {
    std::unique_ptr<msg> msg_to_send = msg_factory->new_message_by_name("data");
    std::unique_ptr<msg> msg_type = msg_factory->new_message_by_name("blocks_response");
    std::string data, serialized_block;
    // LOG(DEBUG) << id << " create:: before loop";
    for (uint32_t i = 0; i < hashes.size(); ++i) {
      global_state_mutex.lock();
      serialized_block = global_state->get(hashes[i]);
      global_state_mutex.unlock();
      if (serialized_block == "") {
        continue;
      }
      msg_type->set_repeated_blob(1, hashes[i]);
      msg_type->set_repeated_blob(2, serialized_block);
    }
    // LOG(DEBUG) << id << " create:: after loop";
    msg_to_send->set_message(2, *msg_type);
    msg_to_send->serialize_message(&data);
    return data;
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << e.what();
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
  } catch (...) {
    LOG(ERROR) << el::base::debug::StackTrace();
  }
  return "";
}

std::string node::create_request_blocks_message(std::vector<std::string> hashes) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  try {
    std::unique_ptr<msg> msg_to_send = msg_factory->new_message_by_name("data");
    std::unique_ptr<msg> msg_type = msg_factory->new_message_by_name("blocks_request");
    std::string data;
    for (uint32_t i = 0; i < hashes.size(); ++i) {
      msg_type->set_repeated_blob(2, hashes[i]);
    }
    msg_type->set_blob(1, get_top());
    msg_to_send->set_message(1, *msg_type);
    msg_to_send->serialize_message(&data);
    return data;
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << e.what();
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
  } catch (...) {
    LOG(ERROR) << el::base::debug::StackTrace();
  }
  return "";
}

node::block node::msg_to_block(core::data::msg* m) const {
  return block(m->get_blob(1), m->get_blob(2), m->get_uint32(3), m->get_blob(4), m->get_blob(5));
}

std::unique_ptr<msg> node::block_to_msg(const block& b) const {
  std::unique_ptr<msg> message = msg_factory->new_message_by_name("block");
  message->set_blob(1, b.hash);
  message->set_blob(2, b.prev_hash);
  message->set_uint32(3, b.height);
  message->set_blob(4, b.miner);
  message->set_blob(5, b.nonce);
  return message;
}

std::string node::hash_block(const block& block_) const {
  uint8_t hash[HASH_SIZE];
  core::crypto::hash_transformation* hasher = hash_transformation::create("SHA256");
  hasher->restart();
  hasher->update(reinterpret_cast<const uint8_t*>(block_.prev_hash.c_str()), HASH_SIZE);
  hasher->update(reinterpret_cast<const uint8_t*>(&block_.height), 4);
  hasher->update(reinterpret_cast<const uint8_t*>(block_.miner.c_str()), block_.miner.size());
  hasher->update(reinterpret_cast<const uint8_t*>(block_.nonce.c_str()), HASH_SIZE);
  hasher->final(hash);
  // delete hasher
  return std::string(hash, hash + HASH_SIZE);
}

bool node::is_hash_valid(uint8_t* hash, uint8_t required_leading_zeros) {
  uint8_t current = 0;
  while (required_leading_zeros - 8 >= 0) {
    if (hash[current] != 0) {
      return false;
    }
    current++;
    required_leading_zeros -= 8;
  }
  if (required_leading_zeros > 0) {
    return (hash[current] & ((1 << ( 8 - required_leading_zeros)) - 1)) == hash[current];
  } else {
    return true;
  }
}

void node::increase_nonce() {
  uint32_t current = HASH_SIZE - 1;
  while (nonce[current] == 255) {
    nonce[current] = 0;
    current--;
    if (current < 0) {
      current = HASH_SIZE - 1;
    }
  }
  nonce[current]++;
}

void node::mine(uint32_t number_tries, uint32_t required_leading_zeros) {
  std::string previous_hash;
  uint32_t current_height;
  uint8_t* next_block_hash = new uint8_t[HASH_SIZE]();
  // LOG(DEBUG) << id << " mining: 0";
  std::pair<uint32_t, std::string> res = get_height_and_top();
  current_height = res.first + 1;
  previous_hash = res.second;
  // // LOG(DEBUG) << id << " mining: 0.1";
  for (uint32_t i = 0; i < number_tries; ++i) {
    increase_nonce();
    std::memset(next_block_hash, 255, HASH_SIZE);
    hasher->restart();
    hasher->update(reinterpret_cast<const uint8_t*>(previous_hash.c_str()), HASH_SIZE);
    hasher->update(reinterpret_cast<const uint8_t*>(&current_height), 4);
    hasher->update(reinterpret_cast<const uint8_t*>(id.c_str()), id.size());
    hasher->update(nonce, HASH_SIZE);
    hasher->final(next_block_hash);
    // // LOG(DEBUG) << id << " mining loop: " << i << " FOUND " <<
    //     core::io::string_to_hex(std::string(reinterpret_cast<char*>(next_block_hash), HASH_SIZE))
    //     << " with NONCE " <<
    //     core::io::string_to_hex(std::string(reinterpret_cast<char*>(nonce), HASH_SIZE));
    // // LOG(DEBUG) << id << " is mining " << i;
    if (is_hash_valid(next_block_hash, required_leading_zeros)) {
      std::string new_hash(reinterpret_cast<char*>(next_block_hash), HASH_SIZE);
      global_state_mutex.lock();
      height_mutex.lock();
      chain_top_mutex.lock();
      if (height != current_height - 1) {
        LOG(INFO) << "Newly found block has become invalid!";
        chain_top_mutex.unlock();
        height_mutex.unlock();
        global_state_mutex.unlock();
        return;
        // TODO(kari): Start over
      }
      // LOG(DEBUG) << "new block is found";
      block b(new_hash, chain_top, ++height, id,
          std::string(reinterpret_cast<char*>(nonce), HASH_SIZE));
      LOG(INFO) << id << " MINED NEW BLOCK:" << b.to_string();
      std::string hash = hash_block(b);
      if (hash != new_hash) {
        chain_top_mutex.unlock();
        height_mutex.unlock();
        global_state_mutex.unlock();
        std::stringstream msg;
        msg << "Hashing doesn't work! " << hash << " vs " << new_hash;
        LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
        throw std::runtime_error(msg.str());
      }
      std::string serialized_block;
      std::unique_ptr<msg> block_msg = block_to_msg(b);
      // LOG(DEBUG) << "Block msg: " << block_msg->to_string();
      block_msg->serialize_message(&serialized_block);
      // LOG(DEBUG) << id << " ADDING BLOCK: " << automaton::core::io::string_to_hex(hash);
      global_state->set(hash, serialized_block);
      chain_top = hash;
      chain_top_mutex.unlock();
      height_mutex.unlock();
      global_state_mutex.unlock();
      send_message(create_send_blocks_message({hash}));
    }
  }
  // LOG(DEBUG) << id << " mining: 1";
  delete next_block_hash;
}
}  // namespace examples
}  // namespace automaton
