#include "automaton/core/script/lua/lua_script_engine.h"

#include <iomanip>

#include "automaton/core/io/io.h"

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

static void msg_to_lua_table(lua_State* L, data::msg* m) {
}

static void lua_to_msg(lua_State* L, int start_param, int params_num, data::msg* input_msg) {
  auto& factory = automaton::core::script::registry::instance().get_factory();
  uint32_t input_schema_id = input_msg->get_schema_id();
  uint32_t fields_number = factory.get_fields_number(input_schema_id);
  if (fields_number < params_num) {
    LOG(ERROR) << factory.get_schema_name(input_schema_id);
    LOG(ERROR) << "Expecting " << fields_number << " got " << params_num;
    lua_pushstring(L, "Too many arguments!");
    lua_error(L);
  }
  for (uint32_t i = 0; i < params_num; i++) {
    auto param = start_param + i;
    // VLOG(9) << input_msg->get_message_type() << " Param " << param;
    auto field = factory.get_field_info(input_schema_id, i);
    auto tag = field.tag;
    size_t buf_len = 0;

    switch (lua_type(L, param)) {
      case LUA_TNUMBER: {
        switch (field.type) {
          case schema::int32: {
            input_msg->set_int32(tag, lua_tonumber(L, param));
            break;
          }
          case schema::int64: {
            input_msg->set_int64(tag, lua_tonumber(L, param));
            break;
          }
          case schema::uint32: {
            input_msg->set_uint32(tag, lua_tonumber(L, param));
            break;
          }
          case schema::uint64: {
            input_msg->set_uint64(tag, lua_tonumber(L, param));
            break;
          }
          case schema::blob: {
            input_msg->set_blob(tag, std::to_string(lua_tonumber(L, param)));
            break;
          }
          default: {
            lua_error(L, "Can not convert number to " + field.fully_qualified_type);
          }
        }
        break;
      }
      case LUA_TSTRING: {
        const char* buf = luaL_checklstring(L, param, &buf_len);
        input_msg->set_blob(tag, std::string(buf, buf_len));
        break;
      }
      case LUA_TBOOLEAN: {
        input_msg->set_boolean(tag, lua_toboolean(L, param));
        break;
      }
      case LUA_TUSERDATA: {
        switch (field.type) {
          case schema::blob: {
            std::stringstream ss;
            auto userdata = reinterpret_cast<void**>(lua_touserdata(L, param));
            ss << std::hex << std::setw(2) << std::setfill('0') << *userdata << ' ';
            input_msg->set_blob(tag, ss.str());
            break;
          }
          default: {
            lua_error(L, "Can not convert userdata to " + field.type_name());
          }
        }
        break;
      }
      case LUA_TTABLE:
      case LUA_TFUNCTION:
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
        lua_pushlstring(L, blob.data(), blob.size());
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

static int wrap_static_function(lua_State *L) {
  int params_num = lua_gettop(L);

  auto funcptr = lua_touserdata(L, lua_upvalueindex(1));
  auto func = reinterpret_cast<module::module_static_function>(funcptr);
  CHECK_NOTNULL(func);
  uint32_t input_schema_id = lua_tonumber(L, lua_upvalueindex(2));
  uint32_t output_schema_id = lua_tonumber(L, lua_upvalueindex(3));
  // auto fname = lua_tostring(L, lua_upvalueindex(4));

  auto& factory = automaton::core::script::registry::instance().get_factory();

  // Prepare function input arguments.
  auto input_msg = factory.new_message_by_id(input_schema_id);
  auto output_msg = factory.new_message_by_id(output_schema_id);
  lua_to_msg(L, 1, params_num, input_msg.get());

  // Call static module function.
  auto status = func(*input_msg.get(), output_msg.get());

  if (status.code != status::OK) {
    LOG(ERROR) << "Scripting error: " << status.msg;
    lua_pushstring(L, status.msg.c_str());
    lua_error(L);
  }

  // Push output result onto the stack.
  return msg_to_lua(L, output_msg.get());
}

static int wrap_object_constructor(lua_State *L) {
  int params_num = lua_gettop(L);

  auto funcptr = lua_touserdata(L, lua_upvalueindex(1));
  auto constructor = reinterpret_cast<module::constructor_function>(funcptr);
  CHECK_NOTNULL(constructor);
  uint32_t constructor_schema_id = lua_tonumber(L, lua_upvalueindex(2));
  auto type_name = lua_tostring(L, lua_upvalueindex(3));

  auto& factory = automaton::core::script::registry::instance().get_factory();

  // Prepare function input arguments.
  auto constructor_msg = factory.new_message_by_id(constructor_schema_id);
  lua_to_msg(L, 1, params_num, constructor_msg.get());

  // Call static module function.
  common::obj* result = constructor(*constructor_msg.get()).release();
  void* userdata = lua_newuserdata(L, sizeof(result));
  memcpy(userdata, &result, sizeof(void*));

  // Set class metatable.
  luaL_getmetatable(L, type_name);
  lua_setmetatable(L, -2);

  return 1;
}

static int wrap_object_method_call(lua_State *L) {
  int params_num = lua_gettop(L);
  uint32_t input_schema_id = lua_tonumber(L, lua_upvalueindex(1));
  uint32_t output_schema_id = lua_tonumber(L, lua_upvalueindex(2));
  auto class_name = lua_tostring(L, lua_upvalueindex(3));

  luaL_checktype(L, 1, LUA_TUSERDATA);
  auto object_ptr = reinterpret_cast<common::obj**>(luaL_checkudata(L, 1, class_name));
  if (object_ptr == NULL) {
    return luaL_argerror(L, 1, "Wrong userdata class");
  }
  auto object = *object_ptr;

  // Prepare method input arguments.
  auto& factory = automaton::core::script::registry::instance().get_factory();
  auto input_msg = factory.new_message_by_id(input_schema_id);
  auto output_msg = factory.new_message_by_id(output_schema_id);
  lua_to_msg(L, 2, params_num - 1, input_msg.get());

  // Call object method.
  auto status = object->process(*input_msg.get(), output_msg.get());

  if (status.code != status::OK) {
    LOG(ERROR) << "Scripting error: " << status.msg;
    lua_pushstring(L, status.msg.c_str());
    lua_error(L);
  }

  // Push output result onto the stack.
  return msg_to_lua(L, output_msg.get());
}

lua_script_engine::lua_script_engine() {
  LOG(DEBUG) << "Creating script engine";
  lua.open_libraries();
  L = lua.lua_state();
  // luaL_openlibs(L);
  // bind_registered_modules();
}

lua_script_engine::~lua_script_engine() {
  LOG(DEBUG) << "Destroying script engine";
}

void lua_script_engine::bind_static_function(module* m,
                                             const module::static_function_info& info) {
  // VLOG(9) << "Binding " << m->name() << "." << info.name
  //     << "<" << info.input_schema_id << ", " << info.output_schema_id << ">";
  lua_pushlightuserdata(L, reinterpret_cast<void*>(info.func));
  lua_pushnumber(L, info.input_schema_id);
  lua_pushnumber(L, info.output_schema_id);
  // lua_pushstring(L, info.name.c_str());
  lua_pushcclosure(L, &wrap_static_function, 3);

  lua_setglobal(L, info.name.c_str());
}

void lua_script_engine::bind_class(module* m,
                                   const module::implementation_info& info) {
  // VLOG(9) << "Binding " << m->name() << "." << info.name
  //     << "<" << info.constructor_schema_id << ">";

  // Create class constructor.
  lua_pushlightuserdata(L, reinterpret_cast<void*>(info.func));
  lua_pushnumber(L, info.constructor_schema_id);
  lua_pushstring(L, info.name.c_str());
  lua_pushcclosure(L, &wrap_object_constructor, 3);
  lua_setglobal(L, info.name.c_str());

  // Create class metatable.
  luaL_newmetatable(L, info.name.c_str());

  // Bind methods
  lua_pushstring(L, "__index");
  lua_newtable(L);
  for (auto& concept : info.concepts) {
    for (auto& method : concept.methods) {
      VLOG(9) << method.name
          << "<" << method.input_schema_id
          << ", " << method.output_schema_id << ">";
      // Method name
      lua_pushstring(L, method.name.c_str());

      // Function with up-values.
      lua_pushnumber(L, method.input_schema_id);
      lua_pushnumber(L, method.output_schema_id);
      lua_pushstring(L, info.name.c_str());
      lua_pushcclosure(L, &wrap_object_method_call, 3);

      // methods_table.method_name = process function
      lua_settable(L, -3);
    }
  }

  // metatable.__index = methods_table.
  lua_rawset(L, -3);

  lua_pop(L, 1);
}

void lua_script_engine::bind_registered_module(module* m) {
  // VLOG(9) << "Binding module " << m->name();
  for (auto& func : m->functions()) {
    bind_static_function(m, func.second);
  }
  for (auto& impl : m->implementations()) {
    // VLOG(9) << "Binding class " << impl.first;
    bind_class(m, impl.second);
  }
}

void lua_script_engine::bind_registered_modules() {
  auto& r = automaton::core::script::registry::instance();
  for (auto& name : r.module_names()) {
    auto m = r.get_module(name);
    bind_registered_module(m);
  }
}

void lua_script_engine::bind_io() {
  lua.set_function("hex", [](const std::string& s) {
    return io::bin2hex(s);
  });
}

void lua_script_engine::bind_log() {
}

void lua_script_engine::bind_network() {
}

void lua_script_engine::bind_state() {
}

}  // namespace lua
}  // namespace script
}  // namespace core
}  // namespace automaton
