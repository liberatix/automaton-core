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

}  // namespace smartproto
}  // namespace core
}  // namespace automaton
