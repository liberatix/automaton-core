#include "automaton/core/smartproto/node.h"

#include <regex>

#include "automaton/core/data/protobuf/protobuf_factory.h"

using automaton::core::data::msg;
using automaton::core::data::protobuf::protobuf_factory;

using std::make_unique;
using std::unique_ptr;
using std::string;

namespace automaton {
namespace core {
namespace smartproto {

// TODO(kari): Clear buffer in info on erase/delete/destruct

static const uint32_t MAX_MESSAGE_SIZE = 512;  // Maximum size of message in bytes
static const uint32_t HEADER_SIZE = 3;
static const uint32_t WAITING_HEADER = 1;
static const uint32_t WAITING_MESSAGE = 2;

node::node(unique_ptr<data::schema> schema, const string& lua_script)
    : peer_ids(0)
    , msg_factory(make_unique<protobuf_factory>())
    , lua(script_engine.get_sol())
    , acceptor_(nullptr) {
  LOG(DEBUG) << "Node constructor called";
  msg_factory->import_schema(schema.get(), "", "");
  script_engine.bind_core();

  // Bind schema messages.
  for (uint32_t id = 0; id < msg_factory->get_schemas_number(); id++) {
    auto name = msg_factory->get_schema_name(id);
    LOG(DEBUG) << "Binding proto message " << name;

    lua.set(name, [this, name, id]() -> unique_ptr<msg> {
      return this->msg_factory->new_message_by_id(id);
    });
  }

  sol::protected_function_result pfr =
      lua.safe_script(lua_script, &sol::script_pass_on_error);
  std::string output = pfr;
  std::cout << output << std::endl;
}

node::~node() {
  LOG(DEBUG) << "Node destructor called";
  std::vector<peer_id> res = list_known_peers();
  LOG(DEBUG) << "Known peers " << res.size();
  for (uint32_t i = 0; i < res.size(); ++i) {
    LOG(DEBUG) << "known_peer: " << res[i];
  }
}

void node::script(const char* input) {
  LOG(DEBUG) << "Calling script from node " << input;
  std::string cmd{input};
  sol::protected_function_result pfr = lua.safe_script(cmd, &sol::script_pass_on_error);
  std::string output = pfr;
  std::cout << output << std::endl;
}

peer_info node::get_peer_info(peer_id id) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = known_peers.find(id);
  if (it != known_peers.end()) {
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
    return it->second;
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  return peer_info{0};
}

bool node::set_peer_info(peer_id id, const peer_info& info) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = known_peers.find(id);
  if (it != known_peers.end()) {
    it->second = info;
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
    return true;
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  return false;
}

// void node::send_message(peer_id id, const core::data::msg& message) {}

void node::send_message(peer_id id, const std::string& message) {
  LOG(DEBUG) << (acceptor_ ? acceptor_->get_address() : "N/A") << " send message " << core::io::bin2hex(message) <<
      " to peer " << id;
  uint32_t message_size = message.size();
  if (message_size > MAX_MESSAGE_SIZE) {
    LOG(ERROR) << "Message size is " << message_size << " and is too big! Max message size is " << MAX_MESSAGE_SIZE;
    return;
  }
  char buffer[3];
  buffer[2] = message_size & 0xff;
  buffer[1] = (message_size >> 8) & 0xff;
  buffer[0] = (message_size >> 16) & 0xff;
  // TODO(kari): Find more effective way to do this
  std::string new_message = std::string(buffer, 3) + message;
  std::lock_guard<std::mutex> lock(peers_mutex);
  if (connected_peers.find(id) == connected_peers.end()) {
    LOG(ERROR) << "Peer " << id << " is not connected! Call connect first!";
    return;
  }
  auto it = known_peers.find(id);
  if (it == known_peers.end()) {
    LOG(ERROR) << "Trying to send message to unknown peer " << id;
    return;
  }
  if (it->second.connection) {
    if (it->second.connection->get_state() == core::network::connection::state::connected) {
      it->second.connection->async_send(new_message);
    }
  } else {
    LOG(ERROR) << "No connection in peer " << id;
  }
}

bool node::connect(peer_id id) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  std::lock_guard<std::mutex> lock(peers_mutex);
  if (connected_peers.find(id) != connected_peers.end()) {
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
    LOG(DEBUG) << "Peer " << id << " is already connected!";
    return false;
  }
  auto it = known_peers.find(id);
  if (it != known_peers.end()) {
    if (!it->second.connection) {
      LOG(ERROR) << "Connection does not exist!";
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
      return false;
    }
    if (it->second.connection->get_state() == core::network::connection::state::disconnected) {
      it->second.connection->connect();
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
      return true;
    }
  } else {
    LOG(ERROR) << "No such peer " << id;
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  return false;
}

bool node::disconnect(peer_id id) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it1 = connected_peers.find(id);
  if (it1 != connected_peers.end()) {
    auto it = known_peers.find(id);
    if (it != known_peers.end()) {
      it->second.connection->disconnect();
    } else {
      // not in known peers
    }
    connected_peers.erase(it1);
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
    return true;
  } else {
    LOG(ERROR) << "Peer " << id << " is not connected!";
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  return false;
}

bool node::set_acceptor(const char* address) {
  core::network::acceptor* new_acceptor = nullptr;
  try {
    std::string protocol, addr;
    if (!address_parser(address, &protocol, &addr)) {
      LOG(DEBUG) << "Address was not parsed!";
      return false;
    }
    new_acceptor = core::network::acceptor::create(protocol, addr, this, this);
    if (new_acceptor && !new_acceptor->init()) {
      LOG(DEBUG) << "Acceptor initialization failed! Acceptor was not created!" << address;
      delete new_acceptor;
      return false;
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "Adding acceptor failed! " << address << " Error: " << e.what();
    if (new_acceptor) {
      delete new_acceptor;
    }
    return false;
  }
  if (!new_acceptor) {
    LOG(ERROR) << "Acceptor was not created!";
    return false;
  }
  acceptor_ = std::shared_ptr<core::network::acceptor> (new_acceptor);
  new_acceptor->start_accepting();
  return true;
}

peer_id node::add_peer(const std::string& address) {
  // TODO(kari): Return 0 on error?
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  std::lock_guard<std::mutex> lock(peers_mutex);
  for (auto it = known_peers.begin(); it != known_peers.end(); ++it) {
    if (it->second.address == address) {
      LOG(ERROR) << "Already have peer " << address;
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
      return it->first;
    }
  }
  peer_info info;
  info.address = address;
  info.id = get_next_peer_id();
  info.connection = nullptr;
  info.buffer = new char[MAX_MESSAGE_SIZE];
  core::network::connection* new_connection = nullptr;
  try {
    std::string protocol, addr;
    if (!address_parser(address, &protocol, &addr)) {
      LOG(DEBUG) << "Address was not parsed! " << address;
    } else {
      new_connection = core::network::connection::create(protocol, info.id, addr, this);
      if (new_connection && !new_connection->init()) {
        LOG(DEBUG) << "Connection initialization failed! Connection was not created!";
        delete new_connection;
      }
    }
  } catch (std::exception& e) {
    LOG(ERROR) << e.what();
    if (new_connection) {
      delete new_connection;
    }
  }
  if (!new_connection) {
    LOG(DEBUG) << "No new connection";
  } else {
    info.connection = std::shared_ptr<core::network::connection> (new_connection);
  }
  known_peers[info.id] = info;
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  return info.id;
}

void node::remove_peer(peer_id id) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it1 = known_peers.find(id);
  if (it1 != known_peers.end()) {
    auto it2 = connected_peers.find(id);
    if (it2 != connected_peers.end()) {
      it1->second.connection->disconnect();
      connected_peers.erase(it2);
    }
    known_peers.erase(it1);
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
}

std::vector<peer_id> node::list_known_peers() {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A");
  std::lock_guard<std::mutex> lock(peers_mutex);
  std::vector<peer_id> res;
  for (auto it = known_peers.begin(); it != known_peers.end(); ++it) {
    res.push_back(it->first);
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A");
  return res;
}

std::set<peer_id> node::list_connected_peers() {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A");
  std::lock_guard<std::mutex> lock(peers_mutex);
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A");
  return connected_peers;
}

peer_id node::get_next_peer_id() {
  std::lock_guard<std::mutex> lock(peer_ids_mutex);
  return ++peer_ids;
}

bool node::address_parser(const std::string& s, std::string* protocol, std::string* address) {
  std::regex rgx_ip("(.+)://(.+)");
  std::smatch match;
  if (std::regex_match(s.begin(), s.end(), match, rgx_ip) &&
      match.size() == 3) {
    *protocol = match[1];
    *address = match[2];
    return true;
  } else {
    LOG(DEBUG) << "match size: " << match.size();
    for (uint32_t i = 0; i < match.size(); i++) {
      LOG(DEBUG) << "match " << i << " -> " << match[i];
    }
    *protocol = "";
    *address = "";
    return false;
  }
}

void node::on_message_received(peer_id c, char* buffer, uint32_t bytes_read, uint32_t id) {
  LOG(DEBUG) << "RECEIVED: " << core::io::bin2hex(std::string(buffer, bytes_read)) << " from peer " << c;
  switch (id) {
    case WAITING_HEADER: {
      if (bytes_read != HEADER_SIZE) {
        LOG(ERROR) << "Wrong header size received";
        return;
      }
      // TODO(kari): make this loop
      uint32_t message_size = 0;
      message_size += (buffer[2] & 0x000000ff);
      message_size += ((buffer[1] & 0x000000ff) << 8);
      message_size += ((buffer[0] & 0x000000ff) << 16);
      LOG(DEBUG) << "MESSAGE SIZE: " << message_size;
      if (!message_size || message_size > MAX_MESSAGE_SIZE) {
        LOG(ERROR) << "Invalid message size!";
        return;
      } else {
        std::lock_guard<std::mutex> lock(peers_mutex);
        auto it = known_peers.find(c);
        if (it != known_peers.end() && it->second.connection && it->second.connection->get_state() ==
            core::network::connection::state::connected) {
          LOG(DEBUG) << (acceptor_ ? acceptor_->get_address() : "N/A") << " waits message with size " << message_size
              << " from peer " << id;
          it->second.connection->async_read(buffer, MAX_MESSAGE_SIZE, message_size, WAITING_MESSAGE);
        }
      }
    }
    break;
    case WAITING_MESSAGE: {
      std::string message = std::string(buffer, bytes_read);
      std::lock_guard<std::mutex> lock(peers_mutex);
      auto it = known_peers.find(c);
      if (it != known_peers.end() && it->second.connection && it->second.connection->get_state() ==
          core::network::connection::state::connected) {
        LOG(DEBUG) << (acceptor_ ? acceptor_->get_address() : "N/A") << " received message " <<
            core::io::bin2hex(message) << " from peer " << c;
        it->second.connection->async_read(buffer, MAX_MESSAGE_SIZE, HEADER_SIZE, WAITING_HEADER);
        s_on_message_received(c, message);
      }
    }
    break;
    default: {}
  }
}

void node::on_message_sent(peer_id c, uint32_t id, core::network::connection::error e) {
  LOG(DEBUG) << c << " -> on_message_sent";
}

void node::on_connected(peer_id c) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c;
  peers_mutex.lock();
  auto it = known_peers.find(c);
  if (it == known_peers.end()) {
    LOG(ERROR) << "Connected to unknown peer " << c << " THIS SHOULD NEVER HAPPEN";
    peers_mutex.unlock();
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c
        << (it->second.address);
    return;
  }
  LOG(DEBUG) << "Connected to " << c;
  connected_peers.insert(c);
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c
      << (it->second.address);
  it->second.connection->async_read(it->second.buffer, MAX_MESSAGE_SIZE, HEADER_SIZE, WAITING_HEADER);
  peers_mutex.unlock();
  s_on_connected(c);
}

void node::on_disconnected(peer_id c) {
  LOG(DEBUG) << c << " -> on_disconnected";
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c;
  peers_mutex.lock();
  auto it = connected_peers.find(c);
  // If the address is not the id
  if (it != connected_peers.end()) {
    connected_peers.erase(it);
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c;
    peers_mutex.unlock();
    s_on_disconnected(c);
  } else {
    LOG(ERROR) << "No such peer " << c;
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c;
    peers_mutex.unlock();
  }
}

void node::on_error(peer_id c, core::network::connection::error e) {
  LOG(DEBUG) << c << " -> on_error";
}

bool node::on_requested(core::network::acceptor* a, const std::string& address, peer_id* id) {
  LOG(DEBUG) << "Requested connection to " << acceptor_->get_address() << " from " << address;
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  std::lock_guard<std::mutex> lock(peers_mutex);
  for (auto it = known_peers.begin(); it != known_peers.end(); ++it) {
    if (it->second.address == address) {
      LOG(ERROR) << "Already have peer " << address;
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
      return false;
    }
  }
  *id = get_next_peer_id();
  peer_info info;
  info.address = address;
  info.id = *id;
  info.connection = nullptr;
  info.buffer = new char[MAX_MESSAGE_SIZE];
  known_peers[*id] = info;
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  return true;
}

void node::on_connected(core::network::acceptor* a, core::network::connection* c, const std::string& address) {
  peer_id id = c->get_id();
  LOG(DEBUG) << "Connected in acceptor " << a->get_address() << " peer with id " << id << " (" << address << ')';
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  peers_mutex.lock();
  auto it = known_peers.find(id);
  if (it == known_peers.end()) {
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
    peers_mutex.unlock();
    LOG(ERROR) << "Connected to unknown peer " << id << " (" << address << ')' << " THIS SHOULD NEVER HAPPEN";
    return;
  }
  it->second.connection = std::shared_ptr<core::network::connection> (c);
  LOG(DEBUG) << "Connected to " << address;
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  peers_mutex.unlock();
}

void node::on_error(core::network::acceptor* a, core::network::connection::error e)  {
  LOG(DEBUG) << a->get_address() << " -> on_error";
}

}  // namespace smartproto
}  // namespace core
}  // namespace automaton
