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
// #include "automaton/core/smartproto/peer.h"

namespace automaton {
namespace core {
namespace smartproto {

typedef core::network::connection_id peer_id;

struct peer_info {
  peer_id id;
  std::string address;
  std::shared_ptr<core::network::connection> connection;
  std::shared_ptr<char> buffer;
  peer_info();
};

class node: public core::network::connection::connection_handler,
    public core::network::acceptor::acceptor_handler {
 public:
  node(std::vector<std::string> schemas,
       std::vector<std::string> lua_scripts,
       std::vector<std::string> wire_msgs);
  ~node();

  peer_info get_peer_info(peer_id id);

  bool set_peer_info(peer_id id, const peer_info& info);

  void send_message(peer_id id, const core::data::msg& msg, uint32_t msg_id);

  void send_blob(peer_id id, const std::string& blob, uint32_t msg_id);

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
  // std::vector<std::unique_ptr<data::schema>> schemas_;
  std::shared_ptr<core::network::acceptor> acceptor_;
  std::mutex peers_mutex;
  std::unordered_map<peer_id, peer_info> known_peers;
  std::set<peer_id> connected_peers;
  std::mutex peer_ids_mutex;

  peer_id get_next_peer_id();

  bool address_parser(const std::string& s, std::string* protocol, std::string* address);

  // Protocol message id map
  std::unordered_map<uint32_t, uint32_t> wire_to_factory;
  std::unordered_map<uint32_t, uint32_t> factory_to_wire;

  // Time based update.
  std::mutex updater_mutex;
  bool updater_stop_signal;
  std::thread* updater;

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
  void s_on_blob_received(peer_id id, const std::string& blob);
  void s_on_connected(peer_id id);
  void s_on_disconnected(peer_id id);

  // Script handler functions.
  std::unordered_map<uint32_t, sol::protected_function> script_on_msg;
  sol::protected_function script_on_connected;
  sol::protected_function script_on_disconnected;
  sol::protected_function script_on_update;
};

}  // namespace smartproto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SMARTPROTO_NODE_H_
