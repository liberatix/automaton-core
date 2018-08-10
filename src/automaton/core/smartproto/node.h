#ifndef AUTOMATON_CORE_SMARTPROTO_NODE_H_
#define AUTOMATON_CORE_SMARTPROTO_NODE_H_

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "automaton/core/data/factory.h"
#include "automaton/core/data/msg.h"
#include "automaton/core/data/schema.h"
#include "automaton/core/network/acceptor.h"
#include "automaton/core/network/connection.h"
#include "automaton/core/script/lua/lua_script_engine.h"

typedef std::string peer_id;

namespace automaton {
namespace core {
namespace smartproto {

struct peer_info {
  peer_id id;
  std::string address;
  std::shared_ptr<core::network::connection> connection;
};

class node: public core::network::connection::connection_handler,
    public core::network::acceptor::acceptor_handler {
 public:
  node(std::unique_ptr<data::schema> schema, const std::string& lua_script);
  ~node();

  peer_info get_peer_info(const peer_id& id);

  bool set_peer_info(const peer_id& id, const peer_info& info);

  void send_message(const peer_id& id, const std::string& msg);

  bool connect(const peer_id& id);

  bool disconnect(const peer_id& id);

  bool set_acceptor(const char* address);

  bool add_peer(const peer_id& id);

  void remove_peer(const peer_id& id);

  std::vector<peer_id> list_known_peers();

  std::set<peer_id> list_connected_peers();

  void script(const char* input);

  uint32_t find_message_id(const char * name) {
    return msg_factory->get_schema_id(name);
  }

  std::unique_ptr<data::msg> create_msg_by_id(uint32_t id) {
    return this->msg_factory->new_message_by_id(id);
  }

 private:
  std::unique_ptr<data::factory> msg_factory;
  script::lua::lua_script_engine script_engine;
  sol::state_view lua;
  std::unique_ptr<data::schema> schema;
  std::shared_ptr<core::network::acceptor> acceptor_;
  std::mutex peers_mutex;
  std::unordered_map<peer_id, peer_info> known_peers;
  std::set<peer_id> connected_peers;

  // Inherited handlers' functions

  void on_message_received(core::network::connection* c, char* buffer,
      uint32_t bytes_read, uint32_t id);

  void on_message_sent(core::network::connection* c, uint32_t id,
      core::network::connection::error e);

  void on_connected(core::network::connection* c);

  void on_disconnected(core::network::connection* c);

  void on_error(core::network::connection* c, core::network::connection::error e);

  bool on_requested(core::network::acceptor* a, const std::string& address);

  void on_connected(core::network::acceptor* a, core::network::connection* c,
      const std::string& address);

  void on_error(core::network::acceptor* a, core::network::connection::error e);

  // Script handler functions.
  void on_message_received(const peer_id& id, const core::data::msg& message) {}
  void on_connected(const peer_id& id) {}
  void on_disconnected(const peer_id& id) {}

  // Cached script handler functions.
  sol::function script_on_msg_received;
  sol::function script_on_connected;
  sol::function script_on_disconnected;
};

}  // namespace smartproto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SMARTPROTO_NODE_H_
