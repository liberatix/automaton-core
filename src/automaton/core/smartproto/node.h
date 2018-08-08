#ifndef AUTOMATON_CORE_SMARTPROTO_NODE_H_
#define AUTOMATON_CORE_SMARTPROTO_NODE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "automaton/core/data/factory.h"
#include "automaton/core/data/msg.h"
#include "automaton/core/data/schema.h"
#include "automaton/core/network/acceptor.h"
#include "automaton/core/network/connection.h"
#include "automaton/core/script/lua/lua_script_engine.h"

typedef uint32_t peer_id;

namespace automaton {
namespace core {
namespace smartproto {

struct peer_info {
  peer_id id;
  std::string address;
  core::network::connection::state status;

  peer_info();
};

class node: public core::network::connection::connection_handler,
    public core::network::acceptor::acceptor_handler {
 public:
  node(std::unique_ptr<data::schema> schema, const std::string& lua_script);

  peer_info get_peer_info(peer_id id);

  bool set_peer_info(peer_id id, const peer_info& info);

  void send_message(peer_id id, const core::data::msg& message);

  bool connect(peer_id id);

  bool disconnect(peer_id id);

  bool set_acceptor(const char* address);

  peer_id add_peer(const char* address);

  void remove_peer(peer_id id);

  std::vector<peer_id> list_known_peers();

  std::vector<peer_id> list_connected_peers();

 private:
  script::lua::lua_script_engine script_engine;
  sol::state_view lua;
  std::unique_ptr<data::factory> msg_factory;
  std::unique_ptr<data::schema> schema;
  core::network::acceptor* acceptor_;
  std::mutex peers_mutex;
  std::unordered_map<peer_id, peer_info> known_peers;
  std::unordered_map<peer_id, core::network::connection*> connected_peers;
  peer_id next_peer_id;
  std::mutex peer_ids_mutex;

  peer_id get_next_peer_id();

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
  void on_message_received(peer_id id, const core::data::msg& message) {}
  void on_connected(peer_id id) {}
  void on_disconnected(peer_id id) {}

  // Cached script handler functions.
  sol::function script_on_msg_received;
  sol::function script_on_connected;
  sol::function script_on_disconnected;
};

}  // namespace smartproto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SMARTPROTO_NODE_H_
