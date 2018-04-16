#include "node/node.h"

#include <mutex>  // NOLINT

#include "network/tcp_implementation.h"

/**

  The connection handlers are used to handle incoming messages and errors. When
  a message is received from a peer to which we are connected through Automaton
  protocol, the handler parses the header, processes it or pass it to other
  protocol handler. If we want to support other protocols (outside automaton
  network) we can set other handlers to these connections. For example if we
  want to connect with peer X on the bitcoin network, we pass something like
  bitcoin_handler which inherits connection_handler and works with bitcoin
  messages.

**/

std::string string_to_hex(const std::string& input) {
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i) {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

std::string thread_id() {
  const std::thread::id id = std::this_thread::get_id();
  static std::size_t nextindex = 0;
  static std::mutex my_mutex;
  static std::map<std::thread::id, std::size_t> ids;
  std::lock_guard<std::mutex> lock(my_mutex);
  if (ids.find(id) == ids.end()) {
    ids[id] = nextindex++;
  }
  return "thread " + std::to_string(ids[id]);
}

protocol::protocol(std::string id_):id(id_) {}
std::string protocol::get_id() {
  return id;
}
protocol::~protocol() {}

peer::peer(const std::string& id_, const std::string& address, int buffer_size_,
    connection::connection_handler* handler): id(id_) {
  waiting_header = true;
  buffer = new char[buffer_size_];
  buffer_size = buffer_size_;
  peer_connection = connection::create("tcp", address, handler);
}
peer::peer(const std::string& id_, connection* connection_, int buffer_size_):
    id(id_) {
  waiting_header = true;
  buffer = new char[buffer_size_];
  buffer_size = buffer_size_;
  peer_connection = connection_;
}
peer::~peer() {
  // delete connection_;
  // connection needs destructor
  peer_connection->disconnect();
  delete buffer;
}
void peer::connect() {
  peer_connection->connect();
}
void peer::disconnect() {
  peer_connection->disconnect();
}
void peer::send_message(const std::string& message) {
  logging(thread_id() + " --> " + "Sending: " + string_to_hex(message));
  peer_connection->async_send(message, 0);
}

void peer::async_read(int num_bytes) {
  peer_connection->async_read(buffer, buffer_size, num_bytes);
}

std::string peer::get_id() {
  return id;
}
void peer::set_id(const std::string& new_id) {
  id = new_id;
}

void peer::add_protocol_state(const std::string& protocol,
    const std::string& state) {
  protocol_states[protocol] = state;
}
void peer::remove_protocol(const std::string& protocol) {
  auto iterator_ = protocol_states.find(protocol);
  if (iterator_ == protocol_states.end()) {
    logging("In remove_protocol(): No such protocol");
    return;
  }
  protocol_states.erase(iterator_);
}
std::string peer::get_protocol_state(const std::string& protocol) {
  auto iterator_ = protocol_states.find(protocol);
  if (iterator_ == protocol_states.end()) {
    logging("In get_protocol_state(): No such protocol");
    return "";
  }
  return iterator_->second;
}

node::node(const std::string& id_, std::vector<protocol*> protocols):
    id(id_) {
  for (unsigned int i = 0; i < protocols.size(); ++i) {
    supported_protocols[protocols[i]->get_id()] = protocols[i];
  }
}
node::~node() {
/*  for (auto it = peers.begin(); it != peers.end(); ++it) {
    delete it->second;
  }
  for (unsigned int i = 0; i < acceptos.size(); ++i) {
    delete acceptors[i];
  }
  for (unsigned int i = 0; i < thread_pool.size(); ++i) {
    thread_pool[i].join();
  }*/
}
bool node::init() {
  try {
    tcp_init();
    acceptor_handler_ = new node_acceptor_handler(this);
    connection_handler_ = new node_connection_handler(this);
  } catch (...) {
    return false;
  }
  return true;
}

void node::add_protocol(protocol* protocol_) {
  supported_protocols[protocol_->get_id()] = protocol_;
}

void remove_protocol(std::string id) {
  // TODO(kari): protocol table need to be thread safe
}

bool node::parse_header(const std::string& message, int* size,
    std::string* protocol_) {
  if (message.size() != 2) {
    logging("Invalid header size");
    return false;
  }
  std::string protocol = std::string(1, message[1]);
  if (supported_protocols.find(protocol) != supported_protocols.end()) {
    *size = (int) message[0];  // NOLINT
    *protocol_ = protocol;
    return true;
  }
  logging(thread_id() + " --> " + "### Unknown protocol message[1]: expected "
      "'A' or 'B', got " + protocol);
  return false;
}
std::string node::add_header(const std::string& message,
    const std::string& protocol) {
  if (protocol.size() < 1) {
    logging("Invalid protocol size in 'add_header()'");
    return "";
  }
  const char header[2] = {static_cast<char>(message.size()), protocol[0]};
  std::string new_message = message;
  return new_message.insert(0, header, 2);
}

void send_message(peer* peer_, protocol* protocol,
    const std::string& message) {
  if (!peer_) {
    logging("In send_message(): No such peer");
  }
  if (!protocol) {
    logging("In send_message(): No such protocol");
  }
}
void broadcast(protocol* protocol, const std::string& message);

const std::string& node::get_id() const {
  return id;
}

void node::add_peer(peer* peer_) {
  if (peer_ && peer_->peer_connection &&
      connection_to_peer.find(peer_->peer_connection) ==
      connection_to_peer.end() && id_to_peer.find(peer_->id) ==
      id_to_peer.end()) {
    peers.push_back(peer_);
    int index = peers.size() - 1;
    connection_to_peer[peer_->peer_connection] = index;
    id_to_peer[peer_->id] = index;
  }
}
void node::remove_peer(std::string id) {
  auto id_iterator = id_to_peer.find(id);
  if (id_iterator == id_to_peer.end()) {
    logging("No peer with id " + id);
    return;
  }
  peer* peer_ = peers[id_iterator->second];
  id_to_peer.erase(id_iterator);
  auto connection_iterator = connection_to_peer.find(peer_->peer_connection);
  if (connection_iterator == connection_to_peer.end()) {
    logging("UNEXPECTED ERROR: No peer with connection " +
        peer_->peer_connection->get_address() + " was saved");
  }
  connection_to_peer.erase(connection_iterator);
  /* TODO(kari): test if this is ok:
  peers[connection_iterator->second].disconnect();*/
  delete peers[connection_iterator->second];
  peers[connection_iterator->second] = nullptr;
  // TODO(kari): Empty and unused spot is created here
}
peer* node::get_peer_by_id(std::string id) {
  auto iterator = id_to_peer.find(id);
  if (iterator == id_to_peer.end()) {
    logging("No peer with id " + id);
    return nullptr;
  }
  return peers[iterator->second];
}
peer* node::get_peer_by_connection(connection* c) {
  auto iterator = connection_to_peer.find(c);
  if (iterator == connection_to_peer.end()) {
    logging("No peer with connection " + c->get_address());
    return nullptr;
  }
  return peers[iterator->second];
}

int node::add_acceptor(acceptor* acceptor_) {
  acceptors.push_back(acceptor_);
  return acceptors.size() - 1;
}
void node::remove_acceptor(int index) {
  // delete acceptors[index]; acceptor need destructor
  acceptors[index] = nullptr;
}

node_connection_handler::node_connection_handler(node* node__):node_(node__) {}
node_connection_handler::~node_connection_handler() {}
void node_connection_handler::on_message_received(connection* c,
    const std::string& message) {
  logging(thread_id() + " --> " + "**Message \"" + string_to_hex(message) + "\""
    "received from " + c ->get_address());
  peer* peer_ = node_->get_peer_by_connection(c);
  if (peer_ == nullptr) {
    logging("Message received from unknown peer" + c->get_address());
    return;
  }
  int message_size;
  if (peer_->waiting_header) {
    if (node_->parse_header(message, &message_size,
        &peer_->waitng_from_protocol)) {
      logging(thread_id() + " --> " + "Message size: " +
          std::to_string(message_size));
      peer_->waiting_header = false;
      peer_->async_read(message_size);
    } else {
      logging(thread_id() + " --> " + "**Bad peer sending invalid data :(");
    }
  } else {  // Waiting message
    if (node_->supported_protocols.find(peer_->waitng_from_protocol) ==
        node_->supported_protocols.end()) {
      logging("Protocol is no longer supported");
      return;
    } else {
      node_->supported_protocols[peer_->waitng_from_protocol]
          ->process_message(message, peer_);
    }
    peer_->waiting_header = true;
    peer_->async_read(2);
  }
}
void node_connection_handler::on_message_sent(connection* c, int id,
    connection::error e) {
  if (e) {
    logging(thread_id() + " --> " + "**Message with id " + std::to_string(id) +
        " was NOT sent to " + c->get_address() + "\nError "
        + std::to_string(e) + " occured");
  } else {
    logging(thread_id() + " --> " + "**Message with id " + std::to_string(id) +
        " was successfully sent to " + c->get_address());
  }
}
void node_connection_handler::on_connected(connection* c) {
  logging(thread_id() + " --> " + "**Connected with: " + c->get_address());
}
void node_connection_handler::on_disconnected(connection* c) {
  logging(thread_id() + " --> " + "**Disconnected with: " + c->get_address());
}
void node_connection_handler::on_error(connection* c, connection::error e) {
  logging(thread_id() + " --> " + "**Error: " + std::to_string(e) +
      " (connection " + c->get_address() + ")");
}

node_acceptor_handler::node_acceptor_handler(node* node__):node_(node__) {}
node_acceptor_handler::~node_acceptor_handler() {}
bool node_acceptor_handler::on_requested(const std::string& address) {
  logging(thread_id() + " --> " + "**Connection required from: " + address +
      " ... Accepting");
  return true;
}
void node_acceptor_handler::on_connected(connection* connection_,
    const std::string& address) {
  logging(thread_id() + " --> " + "**Accepted connection from: " + address);
  peer* peer_ = new peer(address, connection_, 256);  // Default value
  node_->add_peer(peer_);
  peer_->async_read(2);
}
void node_acceptor_handler::on_error(connection::error e) {
  logging(thread_id() + " --> " + "**Error (acceptor): " + std::to_string(e));
}
