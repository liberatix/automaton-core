#include "automaton/core/smartproto/node.h"

#include "automaton/core/data/protobuf/protobuf_factory.h"

using automaton::core::data::protobuf::protobuf_factory;

using std::make_unique;
using std::unique_ptr;
using std::string;

namespace automaton {
namespace core {
namespace smartproto {

node::node(unique_ptr<data::schema> schema, const string& lua_script)
    : msg_factory(make_unique<protobuf_factory>()) {
  msg_factory->import_schema(schema.get(), "", "");
  script_engine.bind_core();
  auto& lua = script_engine.get_sol();
  sol::protected_function_result pfr =
      lua.safe_script(lua_script, &sol::script_pass_on_error);
}

peer_info::peer_info() {}

node::node(std::unique_ptr<data::schema> schema,
     const std::string& lua_script);

peer_info node::get_peer_info(peer_id id) {
  return peer_info();
}

void node::send_message(peer_id id, const core::data::msg& message) {}

bool node::connect(peer_id id) {
  return false;
}

bool node::disconnect(peer_id id) {
  return false;
}

bool node::set_acceptor(const char* address) {
  return false;
}

peer_id node::add_peer(const char* address) {
  return 0;
}

void node::remove_peer(peer_id id) {}

std::vector<peer_id> node::list_known_peers() {
  return std::vector<peer_id> ();
}

std::vector<peer_id> node::list_connected_peers() {
  return std::vector<peer_id> ();
}

void on_message_received(core::network::connection* c, char* buffer,
    uint32_t bytes_read, uint32_t id) {}

void on_message_sent(core::network::connection* c, uint32_t id,
    core::network::connection::error e) {}

void on_connected(core::network::connection* c) {}

void on_disconnected(core::network::connection* c) {}

void on_error(core::network::connection* c, core::network::connection::error e) {}

bool on_requested(core::network::acceptor* a, const std::string& address) {}

void on_connected(core::network::acceptor* a, core::network::connection* c,
    const std::string& address) {}

void on_error(core::network::acceptor* a, core::network::connection::error e) {}

}  // namespace smartproto
}  // namespace core
}  // namespace automaton
