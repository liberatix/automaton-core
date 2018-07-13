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
static const uint32_t MAX_HEADER_SIZE = 255;
static const uint32_t MAX_MESSAGE_SIZE = 512;  // Maximum size of message in bytes
// Header size size is 1 byte

/**
  Next 3 are used as an id in async_read and on_message_received to show at what state is the
  receiving of the message
*/
static const uint32_t WAITING_HEADER_SIZE = 0;
static const uint32_t WAITING_HEADER = 1;
static const uint32_t WAITING_MESSAGE = 2;

core::data::factory* node::msg_factory = nullptr;

// Node's connection handler

void node::on_message_received(connection* c, char* buffer, uint32_t bytes_read, uint32_t id_) {
  // LOG(DEBUG) << id << " RECEIVED: " << core::io::string_to_hex(std::string(buffer, bytes_read));
  switch (id_) {
    case WAITING_HEADER_SIZE: {
      if (bytes_read != 1) {
        std::stringstream msg;
        msg << id << " Reading 1 byte was not successful! Read " << bytes_read << " instead";
        LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
        throw std::runtime_error(msg.str());
      }
      uint32_t s = buffer[0];
      c->async_read(buffer, MAX_MESSAGE_SIZE, s, WAITING_HEADER);
    }
    break;
    case WAITING_HEADER: {
      std::string serialized_header = std::string(buffer, bytes_read);
      std::unique_ptr<msg> header = msg_factory->new_message_by_name("header");
      header->deserialize_message(serialized_header);
      uint32_t message_size = header->get_uint32(1);
      if (!message_size || message_size > MAX_MESSAGE_SIZE) {
        std::stringstream msg;
        msg << id << " Reading header was not successful!";
        LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
        throw std::runtime_error(msg.str());
      } else {
        c->async_read(buffer, MAX_MESSAGE_SIZE, message_size, WAITING_MESSAGE);
      }
    }
    break;
    case WAITING_MESSAGE: {
      try {
        std::string data = std::string(buffer, bytes_read);
        // LOG(DEBUG) << id << " RECEIVED " << data.size() << " bytes: ";
            // << core::io::string_to_hex(data);
        if (data == "") {
          std::stringstream msg;
          msg << id << " Reading message was not successful!";
          LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
          throw std::runtime_error(msg.str());
        }
        std::unique_ptr<msg> received_msg = msg_factory->new_message_by_name("data");
        received_msg->deserialize_message(data);
        process((received_msg->get_message(1)).get(), c);
        process((received_msg->get_message(2)).get(), c);
        c->async_read(buffer, MAX_MESSAGE_SIZE, 1, WAITING_HEADER_SIZE);
      } catch (std::exception& e) {
        std::stringstream msg;
        msg << e.what();
        LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
      } catch (...) {
        LOG(ERROR) << el::base::debug::StackTrace();
      }
    }
    break;
    default: {}
  }
}

void node::on_message_sent(connection* c, uint32_t id_, connection::error e) {
  if (e) {
    LOG(ERROR) << "Message with id " << std::to_string(id_) << " was NOT sent to " <<
        c->get_address() << " -> Error " << std::to_string(e) << " occurred";
  } else {
    // logging("Message with id " + std::to_string(id) + " was successfully sent to " +
    //    c->get_address());
  }
}

void node::on_connected(connection* c) {
  // LOG(INFO) << id << " connected with: " + c->get_address();
  c->async_send(add_header(create_request_blocks_message({})));
  c->async_read(add_buffer(MAX_MESSAGE_SIZE), MAX_MESSAGE_SIZE, 1, WAITING_HEADER_SIZE);
}

void node::on_disconnected(connection* c) {
  // LOG(INFO) << id << " disconnected with: " + c->get_address();
}

void node::on_error(connection* c, connection::error e) {
  if (e == connection::no_error) {
    return;
  }
  // LOG(ERROR) << "Error: " << std::to_string(e) << " (connection " << c->get_address() << ")";
}

// Node's acceptor handler

bool node::on_requested(acceptor* a, const std::string& address) {
  // EXPECT_EQ(address, address_a);
  // LOG(INFO) << id << " received connection request from : " + address;
  return accept_connection(/*address*/);
}

void node::on_connected(acceptor* a, connection* c, const std::string& address) {
  // LOG(INFO) << id << " accepted connection from " << address;
  add_peer(c, address);
}

void node::on_error(acceptor* a, connection::error e) {
  LOG(ERROR) << std::to_string(e);
  remove_acceptor(a->get_address());
}

// Node

node::node(node_params params,
          std::string (*get_randon_peer_address)(node* n, const node_params& params)):id(""),
          get_randon_peer_address(get_randon_peer_address), params(params),
          first_block_hash(std::string(HASH_SIZE, 0)), chain_top(first_block_hash), height(0),
          initialized(false), peer_ids(0) {}

bool node::init() {
  if (initialized) {
    return false;
  }
  try {
    if (!msg_factory) {
      msg_factory = new protobuf_factory();
      protobuf_schema loaded_schema(core::io::get_file_contents(PROTO_FILE));
      LOG(DEBUG) << "SCHEMA::" << loaded_schema.dump_schema();
      msg_factory->import_schema(&loaded_schema, "proto", "");
      SHA256_cryptopp::register_self();
    }
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

// This function is created because the acceptor needs ids for the connections it accepts
uint32_t node::get_next_peer_id() {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peer_ids_mutex);
  return ++peer_ids;
}

bool node::accept_connection() {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  return true;
}

bool node::add_peer(const std::string& connection_type, const std::string& address) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = peers.find(address);
  if (it != peers.end()) {
    LOG(DEBUG) << "Peer with this address already exists!";
    return false;
  }
  connection* new_connection;
  try {
    new_connection = connection::create(connection_type, address, this);
    if (new_connection && !new_connection->init()) {
      LOG(DEBUG) << "Connection initialization failed! Connection was not created!";
      delete new_connection;
      return false;
    }
  } catch (std::exception& e) {
    LOG(ERROR) << e.what();
    return false;
  }
  if (!new_connection) {
    LOG(ERROR) << "Connection was not created!";  // Possible reason: tcp_init was never called
    return false;
  }
  peers[address] = new_connection;
  new_connection->connect();
  return true;
}

bool node::add_peer(automaton::core::network::connection* c, const std::string& address) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = peers.find(address);
  if (it != peers.end() && it->second == c) {
    LOG(DEBUG) << "Peer with this address already exists!";
    return false;
  } else if (it != peers.end()) {
    delete it->second;
    peers.erase(it);
  }
  peers[address] = c;
  return true;
}

void node::remove_peer(const std::string& id_) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = peers.find(id_);
  if (it != peers.end()) {
    peers.erase(it);
  }
}

automaton::core::network::connection* node::get_peer(const std::string& address) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = peers.find(address);
  if (it == peers.end()) {
    return nullptr;
  }
  return it->second;
}

bool node::add_acceptor(const std::string& connection_type, const std::string& address) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(acceptors_mutex);
  auto it = acceptors.find(address);
  if (it != acceptors.end()) {
    LOG(DEBUG) << "Acceptor with this address already exists!";
    return false;
  }
  acceptor* new_acceptor;
  try {
    new_acceptor = acceptor::create(connection_type, address, this, this);
    if (new_acceptor && !new_acceptor->init()) {
      LOG(DEBUG) << "Acceptor initialization failed! Acceptor was not created!";
      delete new_acceptor;
      return false;
    }
  } catch (std::exception& e) {
    LOG(ERROR) << e.what();
    return false;
  }
  if (!new_acceptor) {
    LOG(ERROR) << "Acceptor was not created!";
    return false;
  }
  acceptors[address] = new_acceptor;
  this->id = address;
  new_acceptor->start_accepting();
  return true;
}

void node::remove_acceptor(const std::string& id_) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";

  std::lock_guard<std::mutex> lock(acceptors_mutex);
  auto it = acceptors.find(id_);
  if (it != acceptors.end()) {
    delete it->second;
    acceptors.erase(it);
  }
}

void node::send_message(const std::string& message, const std::string& connection_id) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::string new_message = add_header(message);
  std::lock_guard<std::mutex> lock(peers_mutex);
  if (connection_id == "") {
    for (auto it = peers.begin(); it != peers.end(); ++it) {
      if (it->second->get_state() == connection::state::connected) {
        it->second->async_send(new_message, 0);
      }
    }
  } else {
    connection* peer = get_peer(connection_id);
    if (peer && peer->get_state() == connection::state::connected) {
      peer->async_send(new_message, 0);
    }
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
  // If we don't have the previous block
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
  // If this is the second block
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
  // If the new block is extending the main chain
  if (block_.prev_hash == chain_top || block_.height > height) {
    chain_top = hash;
    height = block_.height;
  }
  // LOG(DEBUG) << id << " checking orphans";
  check_orphans(hash);
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

std::pair<uint32_t, std::string> node::get_height_and_top() const {
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

std::string node::get_top() const {
  std::lock_guard<std::mutex> lock(chain_top_mutex);
  return chain_top;
}

uint32_t node::get_height() const {
  std::lock_guard<std::mutex> lock(height_mutex);
  return height;
}

// Private functions

void node::check_orphans(const std::string& hash) {
  bool erased;
  std::string current_hash = hash;
  do {
    erased = false;
    for (auto it = orphan_blocks.begin(); it != orphan_blocks.end(); ++it) {
      std::string new_hash = it->first;
      block b = it->second;
      if (b.prev_hash == current_hash) {
        if (b.height != height + 1) {
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
        std::string serialized_new_block;
        block_msg->serialize_message(&serialized_new_block);
        global_state->set(new_hash, serialized_new_block);
        orphan_blocks.erase(it);
        if (b.prev_hash == chain_top || b.height > height) {
          chain_top = new_hash;
          height = b.height;
        }
        current_hash = new_hash;
        erased = true;
        break;
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
    msg_to_send->set_message(2, *msg_type);
    msg_to_send->serialize_message(&data);
    if (data.size() > MAX_MESSAGE_SIZE) {
      LOG(ERROR) << "Create send blocks :: Message size is bigger than " << MAX_MESSAGE_SIZE <<
          " -> " << hashes;
    }
    // LOG(DEBUG) << id << " SENDING " << data.size() << "bytes: " << core::io::string_to_hex(data);
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
    if (data.size() > MAX_MESSAGE_SIZE) {
      LOG(ERROR) << "Create request blocks :: Message size is bigger than " << MAX_MESSAGE_SIZE <<
          " -> " << hashes;
    }
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

void node::process(msg* input_message, connection* sender) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  // LOG(DEBUG) << "PROCESS 1";
  if (!input_message) {
    // LOG(DEBUG) << "PROCESS NO MESSAGE";
    return;
  }
  std::string msg_type = msg_factory->get_schema_name(input_message->get_schema_id());
  // LOG(DEBUG) << "PROCESS TYPE " << msg_type;
  if (msg_type == "blocks_request") {
    // LOG(DEBUG) << "PROCESS 1: 1";
    std::vector<std::string> hashes;
    uint32_t hashes_number = input_message->get_repeated_field_size(2);
    if (hashes_number > 0) {
      for (uint32_t i = 0; i < hashes_number; i++) {
        hashes.push_back(input_message->get_repeated_blob(2, i));
      }
    }
    // LOG(DEBUG) << id << " receiving msg: 3";
    std::string top_block_hash = input_message->get_blob(1);
    // LOG(DEBUG) << "TOP :: " << top_block_hash;
    if (top_block_hash != get_top()) {
      hashes.push_back(get_top());
    }
    if (hashes.size() > 0) {
    //  LOG(DEBUG) << "SENDING :: " << hashes;
      sender->async_send(add_header(create_send_blocks_message(hashes)));
    }
  } else if (msg_type == "blocks_response") {
    // LOG(DEBUG) << "PROCESS 2: 1";
    // LOG(DEBUG) << id << " receiving msg: 1";
    uint32_t hashes_number = input_message->get_repeated_field_size(1);
    uint32_t blocks_number = input_message->get_repeated_field_size(2);
    if (hashes_number != blocks_number) {
      // TODO(kari): Handle this situation
      LOG(ERROR) << "Received message contains different number of hashes and blocks";
      // LOG(DEBUG) << "Blocks:: ";
      // for (int i = 0; i < blocks_number; ++i) {
      //   LOG(DEBUG) << "block: " <<
      //       core::io::string_to_hex(msg_type_response->get_repeated_blob(2, i));
      // }
      return;
    }
    for (uint32_t i = 0; i < hashes_number; ++i) {
      // LOG(DEBUG) << id << " receiving msg: loop " << i;
      std::string hash = input_message->get_repeated_blob(1, i);
      global_state_mutex.lock();
      orphan_blocks_mutex.lock();
      if (global_state->get(hash) == "" &&
          orphan_blocks.find(hash) == orphan_blocks.end()) {
        // LOG(DEBUG) << id << " receiving msg: loop " << i << "if";
        orphan_blocks_mutex.unlock();
        global_state_mutex.unlock();
        std::string serialized_block = input_message->get_repeated_blob(2, i);
        // WHY NO node::msg_factory ???
        std::unique_ptr<msg> block_msg = msg_factory->new_message_by_name("block");
        block_msg->deserialize_message(serialized_block);
        block b = msg_to_block(block_msg.get());
         // LOG(DEBUG) << "Received block: " << b.to_string();
        // validate block
        if (!hash.compare(hash_block(b)) && !hash.compare(b.hash)) {
          // LOG(DEBUG) << "Received valid block! Block send to handler!";
          handle_block(hash, b, serialized_block);
        } else {
          LOG(ERROR) << "Block validation failed!";
        }
      } else {
        // LOG(DEBUG) << id << " receiving msg: loop " << i << "else";
        orphan_blocks_mutex.unlock();
        global_state_mutex.unlock();
      }
    }
  }
}

void node::mine(uint32_t number_tries, uint32_t required_leading_zeros) {
  std::string previous_hash;
  uint32_t current_height;
  uint8_t* next_block_hash = new uint8_t[HASH_SIZE]();
  // LOG(DEBUG) << id << " mining: 0";
  std::pair<uint32_t, std::string> res = get_height_and_top();
  current_height = res.first + 1;
  previous_hash = res.second;
  // LOG(DEBUG) << id << " mining: 0.1";
  for (uint32_t i = 0; i < number_tries; ++i) {
    increase_nonce();
    std::memset(next_block_hash, 255, HASH_SIZE);
    hasher->restart();
    hasher->update(reinterpret_cast<const uint8_t*>(previous_hash.c_str()), HASH_SIZE);
    hasher->update(reinterpret_cast<const uint8_t*>(&current_height), 4);
    hasher->update(reinterpret_cast<const uint8_t*>(id.c_str()), id.size());
    hasher->update(nonce, HASH_SIZE);
    hasher->final(next_block_hash);
    // LOG(DEBUG) << id << " mining loop: " << i << " FOUND " <<
    //     core::io::string_to_hex(std::string(reinterpret_cast<char*>(next_block_hash), HASH_SIZE))
    //     << " with NONCE " <<
    //     core::io::string_to_hex(std::string(reinterpret_cast<char*>(nonce), HASH_SIZE));
    // LOG(DEBUG) << id << " is mining " << i;
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

void node::update() {
  acceptors_mutex.lock();
  if (!acceptors.size()) {
    // LOG(ERROR) << "No acceptors in this node!";
  }
  acceptors_mutex.unlock();
  peers_mutex.lock();
  // Check if there are disconnected peers
  for (auto it = peers.begin(); it != peers.end();) {
    if (it->second->get_state() == connection::state::disconnected) {
      peers.erase(it);
    }
    it++;
  }
  int32_t new_peers = params.connected_peers_count - peers.size();
  peers_mutex.unlock();
  if (new_peers && new_peers > 0) {
    for (int32_t i = 0; i < new_peers; ++i) {
      std::string address;
      do {
        address = get_randon_peer_address(this, params);
      } while (address == id || get_peer(address));
      add_peer(params.connection_type, address);
    }
  }
}

std::string node::add_header(const std::string& message) const {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  if (message.size() < 1) {
    return "";
  }
  if (message.size() > MAX_MESSAGE_SIZE) {
    std::stringstream msg;
    msg << "Message size is too big";
    LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
    throw std::runtime_error(msg.str());
  }
  std::string new_message;
  std::unique_ptr<msg> header = msg_factory->new_message_by_name("header");
  // Setting message size
  header->set_uint32(1, message.size());
  header->serialize_message(&new_message);
  uint32_t header_size = new_message.size();
  if (header_size < 1) {
    std::stringstream msg;
    msg << "Header serialization was not successful!";
    LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
    throw std::runtime_error(msg.str());
  }
  if (header_size > MAX_HEADER_SIZE) {
    std::stringstream msg;
    msg << "Header size is too big";
    LOG(ERROR) << msg.str();  // << '\n' << el::base::debug::StackTrace();
    throw std::runtime_error(msg.str());
  }
  new_message.insert(0, 1, header_size);
  return new_message.append(message);
}

void node::print_node_info() const {
  std::lock_guard<std::mutex> acct_lock(acceptors_mutex);
  std::lock_guard<std::mutex> peer_lock(peers_mutex);
  std::stringstream s;
  s << "NODE ID: " << id << " \nacceptors: ";
  for (auto it = acceptors.begin(); it != acceptors.end(); ++it) {
    s << it->first << " ";
  }
  s << "\npeers: ";
  for (auto it = peers.begin(); it != peers.end(); ++it) {
    s << it->first << " ";
  }
  LOG(INFO) << s.str();
}

}  // namespace examples
}  // namespace automaton
