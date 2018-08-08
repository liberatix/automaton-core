#include "automaton/core/script/lua/lua_script_engine.h"

#include "automaton/core/data/msg.h"
#include "automaton/core/data/factory.h"

using automaton::core::data::msg;
using automaton::core::data::factory;
using automaton::core::data::schema;

namespace automaton {
namespace core {
namespace script {
namespace lua {

void lua_script_engine::bind_data() {
  auto msg_type = lua.create_simple_usertype<msg>();

  msg_type.set(sol::meta_function::index,
    [](sol::this_state L, msg& m, std::string key) -> sol::object {
      auto schema_id = m.get_schema_id();
      auto tag_id = m.get_field_tag(key);
      auto fi = m.get_field_info_by_tag(tag_id);
      auto ftype = fi.type;
      switch (ftype) {
        case schema::int32: {
          return sol::object(L, sol::in_place, m.get_int32(tag_id));
        }
        case schema::int64: {
          return sol::object(L, sol::in_place, m.get_int64(tag_id));
        }
        case schema::uint32: {
          return sol::object(L, sol::in_place, m.get_uint32(tag_id));
        }
        case schema::uint64: {
          return sol::object(L, sol::in_place, m.get_uint64(tag_id));
        }
        case schema::blob: {
          return sol::object(L, sol::in_place, m.get_blob(tag_id));
        }
        default: {
          return sol::make_object(L, sol::lua_nil);
        }
      }
      return sol::make_object(L, sol::lua_nil);
    });

  msg_type.set(sol::meta_function::new_index,
    [](sol::this_state L, msg& m, std::string key, sol::object value) {
      auto schema_id = m.get_schema_id();
      auto tag_id = m.get_field_tag(key);
      auto fi = m.get_field_info_by_tag(tag_id);
      auto ftype = fi.type;
      switch (ftype) {
        case schema::int32: {
          int blob = value.as<int>();
          m.set_int32(tag_id, blob);
          break;
        }
        case schema::int64: {
          auto blob = value.as<int64_t>();
          m.set_int64(tag_id, blob);
          break;
        }
/*
        case schema::uint32: {
          auto blob = value.as<uint32_t>();
          m.set_uint32(tag_id, blob);
          break;
        }
        case schema::uint64: {
          auto blob = value.as<uint64_t>();
          m.set_uint64(tag_id, blob);
          break;
        }
        */
        case schema::blob: {
          auto blob = value.as<const char *>();
          m.set_blob(tag_id, blob);
          break;
        }
        default: {
          LOG(ERROR) << "WAT?";
        }
      }
    });

  msg_type.set("set_blob", &msg::set_blob);
  msg_type.set("get_blob", &msg::get_blob);
  msg_type.set("set_repeated_blob", &msg::set_repeated_blob);
  msg_type.set("get_repeated_blob", &msg::get_repeated_blob);

  msg_type.set("get_int32", &msg::get_int32);
  msg_type.set("set_int32", &msg::set_int32);
  msg_type.set("get_repeated_int64", &msg::get_repeated_int32);
  msg_type.set("set_repeated_int64", &msg::set_repeated_int32);

  msg_type.set("get_uint32", &msg::get_uint32);
  msg_type.set("set_uint32", &msg::set_uint32);
  msg_type.set("get_repeated_uint64", &msg::get_repeated_uint32);
  msg_type.set("set_repeated_uint64", &msg::set_repeated_uint32);

  msg_type.set("get_int64", &msg::get_int64);
  msg_type.set("set_int64", &msg::set_int64);
  msg_type.set("get_repeated_int64", &msg::get_repeated_int64);
  msg_type.set("set_repeated_int64", &msg::set_repeated_int64);

  msg_type.set("get_uint64", &msg::get_uint64);
  msg_type.set("set_uint64", &msg::set_uint64);
  msg_type.set("get_repeated_uint64", &msg::get_repeated_uint64);
  msg_type.set("set_repeated_uint64", &msg::set_repeated_uint64);

  msg_type.set("get_bool", &msg::get_boolean);
  msg_type.set("set_bool", &msg::set_boolean);
  msg_type.set("get_repeated_bool", &msg::get_repeated_boolean);
  msg_type.set("set_repeated_bool", &msg::set_repeated_boolean);

  msg_type.set("get_enum", &msg::get_enum);
  msg_type.set("set_enum", &msg::set_enum);
  msg_type.set("get_repeated_enum", &msg::get_repeated_enum);
  msg_type.set("set_repeated_enum", &msg::set_repeated_enum);

  msg_type.set("get_msg", &msg::get_message);
  msg_type.set("set_msg", &msg::set_message);
  msg_type.set("get_repeated_msg", &msg::get_repeated_message);
  msg_type.set("set_repeated_msg", &msg::set_repeated_message);

  msg_type.set("serialize", [](msg& m) {
      std::string s;
      m.serialize_message(&s);
      return s;
    });

  msg_type.set("deserialize", &msg::deserialize_message);

  msg_type.set("to_json", [](msg& m) {
      std::string json;
      m.to_json(&json);
      return json;
    });

  lua.set_usertype("msg", msg_type);
}

}  // namespace lua
}  // namespace script
}  // namespace core
}  // namespace automaton
