#include "automaton/core/script/lua/lua_script_engine.h"

#include <iomanip>

#include "automaton/core/io/io.h"
#include "automaton/core/data/schema.h"

using automaton::core::data::schema;

namespace automaton {
namespace core {
namespace script {
namespace lua {

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

void lua_script_engine::bind_io() {
  lua.set_function("hex", [](const std::string& s) {
    return io::bin2hex(s);
  });
  lua.set_function("bin", [](const std::string& s) {
    return io::hex2bin(s);
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
