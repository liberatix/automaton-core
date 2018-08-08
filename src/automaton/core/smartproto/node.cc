#include "automaton/core/smartproto/node.h"

#include "automaton/core/data/protobuf/protobuf_factory.h"

using automaton::core::data::msg;
using automaton::core::data::protobuf::protobuf_factory;

using std::make_unique;
using std::unique_ptr;
using std::string;

namespace automaton {
namespace core {
namespace smartproto {

node::node(unique_ptr<data::schema> schema, const string& lua_script)
    : msg_factory(make_unique<protobuf_factory>())
    , lua(script_engine.get_sol()) {
  msg_factory->import_schema(schema.get(), "", "");
  script_engine.bind_core();

  // Bind schema messages.
  for (auto id = 0; id < msg_factory->get_schemas_number(); id++) {
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

void node::on_message_received(core::network::connection* c, char* buffer,
    uint32_t bytes_read, uint32_t id) {}

void node::on_message_sent(core::network::connection* c, uint32_t id,
    core::network::connection::error e) {}

void node::on_connected(core::network::connection* c) {
}

void node::on_disconnected(core::network::connection* c) {
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
