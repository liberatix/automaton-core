#include "automaton/examples/network/first_sp.h"

#include "automaton/core/log/log.h"

/// Node's connection handler

node::handler::handler(node* n): node_(n) {}
void node::handler::on_message_received(automaton::core::network::connection* c, char* buffer,
    uint32_t bytes_read, uint32_t id) {
  std::string message = std::string(buffer, bytes_read);
  // logging("Message \"" + message + "\" received in <" + c->get_address() + ">");
  if (std::stoul(message) > node_->height) {
    node_->height = std::stoul(message);
    node_->send_height();
  }
  c -> async_read(buffer, 16, 0, 0);
}
void node::handler::on_message_sent(automaton::core::network::connection* c, uint32_t id,
    automaton::core::network::connection::error e) {
  if (e) {
     LOG(ERROR) << "Message with id " << std::to_string(id) << " was NOT sent to " <<
        c->get_address() << "\nError " << std::to_string(e);
  } else {
    // logging("Message with id " + std::to_string(id) + " was successfully sent to " +
    //    c->get_address());
  }
}
void node::handler::on_connected(automaton::core::network::connection* c) {
  c->async_read(node_->add_buffer(16), 16, 0, 0);
}
void node::handler::on_disconnected(automaton::core::network::connection* c) {
  // logging("Disconnected with: " + c->get_address());
}
void node::handler::on_error(automaton::core::network::connection* c,
    automaton::core::network::connection::error e) {
  if (e == automaton::core::network::connection::no_error) {
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
void node::lis_handler::on_connected(automaton::core::network::connection* c,
    const std::string& address) {
  // logging("Accepted connection from: " + address);
  node_->peers[node_->get_next_peer_id()] = c;
  c->async_read(node_->add_buffer(16), 16, 0, 0);
}
void node::lis_handler::on_error(automaton::core::network::connection::error e) {
  LOG(ERROR) << std::to_string(e);
}

/// Node

node::node() {
  height = 0;
  peer_ids = 0;
  handler_ = new handler(this);
  lis_handler_ = new lis_handler(this);
}
node::~node() {
  for (int i = 0; i < buffers.size(); ++i) {
    delete [] buffers[i];
  }
  // TODO(kari): delete all acceptors and connections
}

char* node::add_buffer(uint32_t size) {
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
  auto it = peers.find(id);
  if (it != peers.end()) {
  //  delete it->second;  /// Delete existing acceptor
  }
  automaton::core::network::connection* new_connection;
  try {
    new_connection = automaton::core::network::connection::create(connection_type, address,
        handler_);
  } catch (std::exception& e) {
    LOG(ERROR) << e.what();
    peers[id] = nullptr;
    return false;
  }
  peers[id] = new_connection;
  new_connection->connect();
  return true;
}
void node::remove_peer(uint32_t id) {
  auto it = peers.find(id);
  if (it != peers.end()) {
    peers.erase(it);
  }
}
bool node::add_acceptor(uint32_t id, const std::string& connection_type,
    const std::string& address) {
  auto it = acceptors.find(id);
  if (it != acceptors.end()) {
    // delete it->second;  /// Delete existing acceptor
  }
  automaton::core::network::acceptor* new_acceptor;
  try {
    new_acceptor = automaton::core::network::acceptor::create(connection_type, address,
        lis_handler_, handler_);
  } catch (std::exception& e) {
    LOG(ERROR) << e.what();
    acceptors[id] = nullptr;
    return false;
  }
  acceptors[id] = new_acceptor;
  new_acceptor->start_accepting();
  return true;
}
void node::remove_acceptor(uint32_t id) {
  auto it = acceptors.find(id);
  if (it != acceptors.end()) {
    acceptors.erase(it);
  }
}
void node::send_height(uint32_t connection_id) {
  if (!connection_id) {
    for (auto it = peers.begin(); it != peers.end(); ++it) {
      // TODO(kari): if connected
      it->second->async_send(std::to_string(height), 0);
    }
  } else {
    peers[connection_id]->async_send(std::to_string(height), 0);
  }
}
