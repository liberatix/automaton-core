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
  auto& factory = automaton::core::script::registry::instance().get_factory();

  for (auto id = 0; id < factory.get_schemas_number(); id++) {
    auto name = factory.get_schema_name(id);

    lua.set(name, [&factory, id]() {
      return factory.new_message_by_id(id);
    });
  }

  /*
  lua.new_usertype<msg>("msg",
    sol::meta_function::index,
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
    },

    sol::meta_function::new_index,
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
        * /
        case schema::blob: {
          auto blob = value.as<const char *>();
          m.set_blob(tag_id, blob);
          break;
        }
        default: {
          LOG(ERROR) << "WAT?";
        }
      }
    },

    "set_blob", &msg::set_blob,
    "get_blob", &msg::get_blob,
    "set_repeated_blob", &msg::set_repeated_blob,
    "get_repeated_blob", &msg::get_repeated_blob,

    "get_int32", &msg::get_int32,
    "set_int32", &msg::set_int32,
    "get_repeated_int64", &msg::get_repeated_int32,
    "set_repeated_int64", &msg::set_repeated_int32,

    "get_uint32", &msg::get_uint32,
    "set_uint32", &msg::set_uint32,
    "get_repeated_uint64", &msg::get_repeated_uint32,
    "set_repeated_uint64", &msg::set_repeated_uint32,

    "get_int64", &msg::get_int64,
    "set_int64", &msg::set_int64,
    "get_repeated_int64", &msg::get_repeated_int64,
    "set_repeated_int64", &msg::set_repeated_int64,

    "get_uint64", &msg::get_uint64,
    "set_uint64", &msg::set_uint64,
    "get_repeated_uint64", &msg::get_repeated_uint64,
    "set_repeated_uint64", &msg::set_repeated_uint64,

    "get_bool", &msg::get_boolean,
    "set_bool", &msg::set_boolean,
    "get_repeated_bool", &msg::get_repeated_boolean,
    "set_repeated_bool", &msg::set_repeated_boolean,

    "get_enum", &msg::get_enum,
    "set_enum", &msg::set_enum,
    "get_repeated_enum", &msg::get_repeated_enum,
    "set_repeated_enum", &msg::set_repeated_enum,

    "get_msg", &msg::get_message,
    "set_msg", &msg::set_message,
    "get_repeated_msg", &msg::get_repeated_message,
    "set_repeated_msg", &msg::set_repeated_message,

    "serialize", [](msg& m) {
      std::string s;
      m.serialize_message(&s);
      return s;
    },

    "deserialize", &msg::deserialize_message,

    "to_json", [](msg& m) {
      std::string json;
      m.to_json(&json);
      return json;
    }
  );  // NOLINT
*/
}

}  // namespace lua
}  // namespace script
}  // namespace core
}  // namespace automaton
