#include "automaton/core/smartproto/node.h"

#include "automaton/core/data/protobuf/protobuf_factory.h"

using automaton::core::data::msg;
using automaton::core::data::protobuf::protobuf_factory;

using std::make_unique;
using std::unique_ptr;
using std::string;

static const peer_id DEFAULT_ID = "";

namespace automaton {
namespace core {
namespace smartproto {

node::node(unique_ptr<data::schema> schema, const string& lua_script)
    : msg_factory(make_unique<protobuf_factory>())
    , lua(script_engine.get_sol()) {
  msg_factory->import_schema(schema.get(), "", "");
  script_engine.bind_core();

  // Bind schema messages.
  for (uint32_t id = 0; id < msg_factory->get_schemas_number(); id++) {
    auto name = msg_factory->get_schema_name(id);
    LOG(DEBUG) << "Binding proto message " << name;

    lua.set(name, [this, name, id]() -> std::unique_ptr<msg> {
      return this->msg_factory->new_message_by_id(id);
    });
  }

  // Bind this node to its own Lua state.
  lua["send"] = [this](peer_id peer, const core::data::msg& msg) {
    this->send_message(peer, msg);
  };

  sol::protected_function_result pfr =
      lua.safe_script(lua_script, &sol::script_pass_on_error);
  std::string output = pfr;
  std::cout << output << std::endl;
}

peer_info::peer_info() {}

peer_info node::get_peer_info(const peer_id& id) {
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = known_peers.find(id);
  if (it != known_peers.end()) {
    return it->second;
  }
  return peer_info();
}

bool node::set_peer_info(const peer_id& id, const peer_info& info) {
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = known_peers.find(id);
  if (it != known_peers.end()) {
    it->second = info;
    return true;
  }
  return false;
}

void node::send_message(const peer_id& id, const core::data::msg& message) {}

bool node::connect(const peer_id& id) {
  std::lock_guard<std::mutex> lock(peers_mutex);
  if (connected_peers.find(id) != connected_peers.end()) {
    return false;
  }
  auto it = known_peers.find(id);
  if (it != known_peers.end()) {
    if (!it->second.connection) {
      core::network::connection* new_connection = nullptr;
      try {
        // parse address
        new_connection = core::network::connection::create("tcp", it->second.address, this);
        if (new_connection && !new_connection->init()) {
        // LOG(DEBUG) << "Connection initialization failed! Connection was not created!";
        delete new_connection;
        return false;
        }
      } catch (std::exception& e) {
      // LOG(ERROR) << e.what();
        if (new_connection) {
          delete new_connection;
        }
        return false;
      }
      if (!new_connection) {
        return false;
      }
      it->second.connection = new_connection;
    }
    if (it->second.connection->get_state() == core::network::connection::state::disconnected) {
      it->second.connection->connect();
      return true;
    }
  }
  return false;
}

bool node::disconnect(const peer_id& id) {
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = connected_peers.find(id);
  if (it != connected_peers.end()) {
    it->second->disconnect();
    return true;
  }
  return false;
}

bool node::set_acceptor(const char* address) {
  if (acceptor_) {
    delete acceptor_;
  }
  core::network::acceptor* new_acceptor;
  try {
    // parse address
    new_acceptor = core::network::acceptor::create("tcp", std::string(address), this, this);
    if (new_acceptor && !new_acceptor->init()) {
      // LOG(DEBUG) << "Acceptor initialization failed! Acceptor was not created!" << address;
      delete new_acceptor;
      return false;
    }
  } catch (std::exception& e) {
    // LOG(ERROR) << "Adding acceptor failed! " << address << " Error: " << e.what();
    if (new_acceptor) {
      delete new_acceptor;
    }
    return false;
  }
  if (!new_acceptor) {
    // LOG(ERROR) << "Acceptor was not created!";
    return false;
  }
  acceptor_ = new_acceptor;
  new_acceptor->start_accepting();
  return true;
}

bool node::add_peer(const peer_id& id) {
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it = known_peers.find(id);
  if (it == known_peers.end()) {
    known_peers[id] = peer_info();
    return true;
  }
  return false;
}

void node::remove_peer(const peer_id& id) {
  std::lock_guard<std::mutex> lock(peers_mutex);
  auto it1 = known_peers.find(id);
  if (it1 != known_peers.end()) {
    known_peers.erase(it1);
  }
  auto it2 = connected_peers.find(id);
  if (it2 != connected_peers.end()) {
    it2->second->disconnect();
    connected_peers.erase(it2);
  }
}

std::vector<peer_id> node::list_known_peers() {
  std::lock_guard<std::mutex> lock(peers_mutex);
  std::vector<peer_id> res;
  for (auto it = known_peers.begin(); it != known_peers.end(); ++it) {
    res.push_back(it->first);
  }
  return res;
}

std::vector<peer_id> node::list_connected_peers() {
  std::lock_guard<std::mutex> lock(peers_mutex);
  std::vector<peer_id> res;
  for (auto it = connected_peers.begin(); it != connected_peers.end(); ++it) {
    res.push_back(it->first);
  }
  return res;
}

void node::on_message_received(core::network::connection* c, char* buffer,
    uint32_t bytes_read, uint32_t id) {}

void node::on_message_sent(core::network::connection* c, uint32_t id,
    core::network::connection::error e) {}

void node::on_connected(core::network::connection* c) {
  peers_mutex.lock();
  peer_id id = DEFAULT_ID;
  if (known_peers.find(c->get_address()) != known_peers.end()) {
    id = c->get_address();
    return;
  } else {
    for (auto it = known_peers.begin(); it != known_peers.end(); ++it) {
      if (it->second.connection == c) {
        id = it->first;
      }
    }
    // If the accepted peer is not in known_peers
    if (id == DEFAULT_ID) {
      id = c->get_address();
      peer_info info;
      info.id = info.address = id;
      info.connection = c;
      known_peers[id] = info;
    }
  }
  connected_peers[id] = c;
  peers_mutex.unlock();
  on_connected(id);
}

void node::on_disconnected(core::network::connection* c) {
  peers_mutex.lock();
  peer_id id;
  auto it = connected_peers.find(c->get_address());
  // If the address is not the id
  if (it == connected_peers.end()) {
    for (auto it = connected_peers.begin(); it != connected_peers.end(); ++it) {
      if (it->second == c) {
        id = it->first;
        connected_peers.erase(it);
        peers_mutex.unlock();
        on_disconnected(id);
        return;
      }
    }
  } else {
    connected_peers.erase(it);
    peers_mutex.unlock();
    on_disconnected(id);
    return;
  }
  peers_mutex.unlock();
}

void node::on_error(core::network::connection* c, core::network::connection::error e) {
}

bool node::on_requested(core::network::acceptor* a, const std::string& address) {
  return true;
}

void node::on_connected(core::network::acceptor* a, core::network::connection* c,
    const std::string& address) {}

void node::on_error(core::network::acceptor* a, core::network::connection::error e) {}

}  // namespace smartproto
}  // namespace core
}  // namespace automaton
