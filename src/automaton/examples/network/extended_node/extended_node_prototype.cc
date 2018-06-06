#include "automaton/examples/network/extended_node/extended_node_prototype.h"

#include "automaton/core/data/schema.h"
#include "automaton/core/data/factory.h"
#include "automaton/core/data/msg.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "automaton/core/data/protobuf/protobuf_msg.h"

#include "automaton/core/io/io.h"
#include "automaton/core/log/log.h"

namespace acn = automaton::core::network;
namespace acd = automaton::core::data;
namespace protobuf = acd::protobuf;

static const char* PROTO_FILE = "automaton/examples/network/extended_node/sp.proto";
// TO BE REMOVED
static uint32_t HEIGHT_TAG = 3;

acd::factory* node::msg_factory = nullptr;

/// Node's connection handler

node::handler::handler(node* n): node_(n) {}
void node::handler::on_message_received(acn::connection* c, char* buffer,
    uint32_t bytes_read, uint32_t id) {
  try {
    std::string data = std::string(buffer, bytes_read);
    std::unique_ptr<acd::msg> message = msg_factory->new_message_by_name("data");
    message->deserialize_message(data);
    uint32_t received_height = message->get_uint32(HEIGHT_TAG);
    if (received_height > node_->height) {
      node_->height = received_height;
      node_->send_height();
    }
    c -> async_read(buffer, 16, 0, 0);
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << e.what();
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
  } catch (...) {
    LOG(ERROR) << el::base::debug::StackTrace();
  }
}
void node::handler::on_message_sent(acn::connection* c, uint32_t id,
    acn::connection::error e) {
  if (e) {
     LOG(ERROR) << "Message with id " << std::to_string(id) << " was NOT sent to " <<
        c->get_address() << "\nError " << std::to_string(e);
  } else {
    // logging("Message with id " + std::to_string(id) + " was successfully sent to " +
    //    c->get_address());
  }
}
void node::handler::on_connected(acn::connection* c) {
  c->async_read(node_->add_buffer(16), 16, 0, 0);
}
void node::handler::on_disconnected(acn::connection* c) {
  // logging("Disconnected with: " + c->get_address());
}
void node::handler::on_error(acn::connection* c,
    acn::connection::error e) {
  if (e == acn::connection::no_error) {
    return;
  }
  LOG(ERROR) << "Error: " << std::to_string(e) << " (connection " << c->get_address() << ")";
}

/// Node's acceptor handler

node::lis_handler::lis_handler(node* n):node_(n) {}
bool node::lis_handler::on_requested(const std::string& address) {
  // EXPECT_EQ(address, address_a);
  // logging("Connection request from: " + address + ". Accepting...");
  return node_->accept_connection(/*address*/);
}
void node::lis_handler::on_connected(acn::connection* c,
    const std::string& address) {
  // logging("Accepted connection from: " + address);
  node_->peers[node_->get_next_peer_id()] = c;
  c->async_read(node_->add_buffer(16), 16, 0, 0);
}
void node::lis_handler::on_error(acn::connection::error e) {
  LOG(ERROR) << std::to_string(e);
}

/// Node

node::node():initialized(false), height(0), peer_ids(0) {}

bool node::init() {
  if (!msg_factory) {
    try {
      msg_factory = new protobuf::protobuf_factory();
      protobuf::protobuf_schema loaded_schema(automaton::core::io::get_file_contents(PROTO_FILE));
      LOG(DEBUG) << "SCHEMA::" << loaded_schema.dump_schema();
      msg_factory->import_schema(&loaded_schema, "proto", "");
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
  handler_ = new handler(this);
  lis_handler_ = new lis_handler(this);
  return initialized = true;
}
node::~node() {
  for (uint32_t i = 0; i < buffers.size(); ++i) {
    delete [] buffers[i];
  }
  // TODO(kari): delete all acceptors and connections
}

char* node::add_buffer(uint32_t size) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  std::lock_guard<std::mutex> lock(buffer_mutex);
  buffers.push_back(new char[size]);
  return buffers[buffers.size() - 1];
}
/// This function is created because the acceptor needs ids for the connections it accepts
uint32_t node::get_next_peer_id() {
  return ++peer_ids;
}
bool node::accept_connection() { return true; }
bool node::add_peer(uint32_t id, const std::string& connection_type, const std::string& address) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  auto it = peers.find(id);
  if (it != peers.end()) {
  //  delete it->second;  /// Delete existing acceptor
  }
  acn::connection* new_connection;
  try {
    new_connection = acn::connection::create(connection_type, address,
        handler_);
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
void node::remove_peer(uint32_t id) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  auto it = peers.find(id);
  if (it != peers.end()) {
    peers.erase(it);
  }
}
bool node::add_acceptor(uint32_t id, const std::string& connection_type,
    const std::string& address) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  auto it = acceptors.find(id);
  if (it != acceptors.end()) {
    // delete it->second;  /// Delete existing acceptor
  }
  acn::acceptor* new_acceptor;
  try {
    new_acceptor = acn::acceptor::create(connection_type, address,
        lis_handler_, handler_);
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
void node::remove_acceptor(uint32_t id) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  auto it = acceptors.find(id);
  if (it != acceptors.end()) {
    acceptors.erase(it);
  }
}
void node::send_height(uint32_t connection_id) {
  CHECK(initialized == true) << "Node is not initialized! Call init() first!";
  try {
    std::string data;
    std::unique_ptr<acd::msg> message = msg_factory->new_message_by_name("data");
    message->set_uint32(HEIGHT_TAG, height);
    message->serialize_message(&data);
    if (!connection_id) {
      for (auto it = peers.begin(); it != peers.end(); ++it) {
        // TODO(kari): if connected
        it->second->async_send(data, 0);
      }
    } else {
      peers[connection_id]->async_send(data, 0);
    }
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << e.what();
    LOG(ERROR) << msg.str() << '\n' << el::base::debug::StackTrace();
  } catch (...) {
    LOG(ERROR) << el::base::debug::StackTrace();
  }
}
