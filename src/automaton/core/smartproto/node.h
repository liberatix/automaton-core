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
#include "automaton/core/smartproto/peer.h"

namespace automaton {
namespace core {
namespace smartproto {


struct peer_info {
  peer_id id = 0;
  std::string address = "";
  std::shared_ptr<core::network::connection> connection;
  char* buffer = nullptr;
};

class node: public core::network::connection::connection_handler,
    public core::network::acceptor::acceptor_handler {
 public:
  node(std::unique_ptr<data::schema> schema, const std::string& lua_script);
  ~node();

  peer_info get_peer_info(peer_id id);

  bool set_peer_info(peer_id id, const peer_info& info);

  // void send_message(peer_id id, const core::data::msg& message);

  void send_message(peer_id id, const std::string& message);

  bool connect(peer_id id);

  bool disconnect(peer_id id);

  bool set_acceptor(const char* address);

  peer_id add_peer(const std::string& address);

  void remove_peer(peer_id id);

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
  peer_id peer_ids;
  std::unique_ptr<data::factory> msg_factory;
  script::lua::lua_script_engine script_engine;
  sol::state_view lua;
  std::unique_ptr<data::schema> schema;
  std::shared_ptr<core::network::acceptor> acceptor_;
  std::mutex peers_mutex;
  std::unordered_map<peer_id, peer_info> known_peers;
  std::set<peer_id> connected_peers;
  std::mutex peer_ids_mutex;

  peer_id get_next_peer_id();

  bool address_parser(const std::string& s, std::string* protocol, std::string* address);

  // Inherited handlers' functions

  void on_message_received(peer_id c, char* buffer, uint32_t bytes_read, uint32_t id);

  void on_message_sent(peer_id c, uint32_t id, core::network::connection::error e);

  void on_connected(peer_id c);

  void on_disconnected(peer_id c);

  void on_error(peer_id c, core::network::connection::error e);

  bool on_requested(core::network::acceptor* a, const std::string& address, peer_id* id);

  void on_connected(core::network::acceptor* a, core::network::connection* c,
      const std::string& address);

  void on_error(core::network::acceptor* a, core::network::connection::error e);

  // Script handler functions.
  void s_on_message_received(peer_id id, const std::string& message) {}
  void s_on_connected(peer_id id) {}
  void s_on_disconnected(peer_id id) {}

  // Cached script handler functions.
  sol::function script_on_msg_received;
  sol::function script_on_connected;
  sol::function script_on_disconnected;
};

}  // namespace smartproto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SMARTPROTO_NODE_H_
