#include "automaton/core/script/lua/lua_script_engine.h"

using automaton::core::common::status;
using automaton::core::data::schema;
using automaton::core::script::module;

namespace automaton {
namespace core {
namespace script {
namespace lua {

static void lua_error(lua_State* L, std::string error_msg) {
  lua_pushstring(L, error_msg.c_str());
  lua_error(L);
}

static void lua_to_msg(lua_State* L, int params_num, data::msg* input_msg) {
  auto& factory = automaton::core::script::registry::instance().get_factory();
  uint32_t input_schema_id = input_msg->get_schema_id();
  uint32_t fields_number = factory.get_fields_number(input_schema_id);
  if (fields_number < params_num) {
    lua_pushstring(L, "Too many arguments!");
    lua_error(L);
  }
  for (uint32_t i = 0; i < params_num; i++) {
    auto field = factory.get_field_info(input_schema_id, i);
    auto tag = field.tag;
    size_t buf_len = 0;

    switch (lua_type(L, i + 1)) {
      case LUA_TNUMBER: {
        switch (field.type) {
          case schema::int32: {
            input_msg->set_int32(tag, lua_tonumber(L, i + 1));
            break;
          }
          case schema::int64: {
            input_msg->set_int64(tag, lua_tonumber(L, i + 1));
            break;
          }
          case schema::uint32: {
            input_msg->set_uint32(tag, lua_tonumber(L, i + 1));
            break;
          }
          case schema::uint64: {
            input_msg->set_uint64(tag, lua_tonumber(L, i + 1));
            break;
          }
          case schema::blob: {
            input_msg->set_blob(tag, std::to_string(lua_tonumber(L, i + 1)));
            break;
          }
          default: {
            lua_error(L, "Can not convert input argument(s) to msg");
          }
        }
        break;
      }
      case LUA_TSTRING: {
        // Notice the i + 1! In Lua counting starts at 1.
        const char * buf = luaL_checklstring(L, i + 1, &buf_len);
        input_msg->set_blob(tag, std::string(buf, buf_len));
        break;
      }
      case LUA_TBOOLEAN: {
        input_msg->set_boolean(tag, lua_toboolean(L, i + 1));
        break;
      }
      case LUA_TTABLE:
      case LUA_TFUNCTION:
      case LUA_TUSERDATA:
      case LUA_TTHREAD:
      case LUA_TLIGHTUSERDATA:
      case LUA_TNONE:
      case LUA_TNIL:
      default: {
        lua_error(L, "Unsuported argument type!");
      }
    }
  }
}

static int msg_to_lua(lua_State* L, data::msg* output_msg) {
  auto& factory = automaton::core::script::registry::instance().get_factory();
  auto output_schema_id = output_msg->get_schema_id();
  int results = factory.get_fields_number(output_schema_id);
  for (uint32_t i = 0; i < results; i++) {
    auto field = factory.get_field_info(output_schema_id, i);
    auto tag = field.tag;

    switch (field.type) {
      case schema::int32: {
        lua_pushinteger(L, output_msg->get_int32(tag));
        break;
      }
      case schema::int64: {
        lua_pushinteger(L, output_msg->get_int64(tag));
        break;
      }
      case schema::uint32: {
        lua_pushinteger(L, output_msg->get_uint32(tag));
        break;
      }
      case schema::uint64: {
        lua_pushinteger(L, output_msg->get_uint64(tag));
        break;
      }
      case schema::blob: {
        auto blob = output_msg->get_blob(tag);
        lua_pushlstring(L, blob.c_str(), blob.size());
        break;
      }
      default: {
        lua_error(L, "Incompatible output!");
      }
    }
  }

  return results;
}

status lua_script_engine::execute(std::string script) {
  if (luaL_dostring(L, script.c_str())) {
    return status::internal(lua_tostring(L, -1));
  }
  return status::ok();
}

int lua_script_engine::wrap_static_function(lua_State *L) {
  int params_num = lua_gettop(L);

  auto funcptr = lua_touserdata(L, lua_upvalueindex(1));
  auto func = reinterpret_cast<module::module_static_function>(funcptr);
  CHECK_NOTNULL(func);
  uint32_t input_schema_id = lua_tonumber(L, lua_upvalueindex(2));
  uint32_t output_schema_id = lua_tonumber(L, lua_upvalueindex(3));
  auto fname = lua_tostring(L, lua_upvalueindex(4));

  auto& factory = automaton::core::script::registry::instance().get_factory();

  // Prepare function input arguments.
  auto input_msg = factory.new_message_by_id(input_schema_id);
  auto output_msg = factory.new_message_by_id(output_schema_id);
  lua_to_msg(L, params_num, input_msg.get());

  // Call static module function.
  std::string in_json, out_json;
  input_msg->to_json(&in_json);
  auto status = func(*input_msg.get(), output_msg.get());
  output_msg->to_json(&out_json);
  LOG(DEBUG) << "LUA CALL " << fname << in_json << " -> " << out_json;

  if (status.code != status::OK) {
    LOG(ERROR) << "Scripting error: " << status.msg;
    lua_pushstring(L, status.msg.c_str());
    lua_error(L);
  }

  // Push output result onto the stack.
  return msg_to_lua(L, output_msg.get());
}

lua_script_engine::lua_script_engine() {
  L = luaL_newstate();
  bind_registered_modules();
}

void lua_script_engine::bind_static_function(module* m,
                                             std::string fname,
                                             const module::static_function_info& func) {
  LOG(INFO) << "Binding " << m->name() << "." << fname
      << "(" << func.input_schema_id << ", " << func.output_schema_id << ")";
  lua_pushlightuserdata(L, reinterpret_cast<void*>(func.func));
  lua_pushnumber(L, func.input_schema_id);
  lua_pushnumber(L, func.output_schema_id);
  lua_pushstring(L, fname.c_str());
  lua_pushcclosure(L, &wrap_static_function, 4);
  lua_setglobal(L, fname.c_str());
}

void lua_script_engine::bind_registered_module(module* m) {
  LOG(INFO) << "Binding module " << m->name();
  for (auto impl : m->implementations()) {
    // LOG(INFO) << impl.first << " " << impl.second.second;
  }
  for (auto func : m->functions()) {
    bind_static_function(m, func.first, func.second);
  }
}

void lua_script_engine::bind_registered_modules() {
  auto& r = automaton::core::script::registry::instance();
  for (auto name : r.module_names()) {
    auto m = r.get_module(name);
    bind_registered_module(m);
  }
}

}  // namespace lua
}  // namespace script
}  // namespace core
}  // namespace automaton
