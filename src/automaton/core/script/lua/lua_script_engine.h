#ifndef AUTOMATON_CORE_SCRIPT_LUA_LUA_SCRIPT_ENGINE_H_
#define AUTOMATON_CORE_SCRIPT_LUA_LUA_SCRIPT_ENGINE_H_

#include <string>

#include "automaton/core/script/registry.h"

#include "lua.hpp"

namespace automaton {
namespace core {
namespace script {
namespace lua {

/**
  Lua script engine wrapper and Autoamaton's module bridge.
*/
class lua_script_engine {
 public:
  /**
  */
  lua_script_engine();

  /**
  */
  void bind_registered_modules();

  /**
  */
  common::status execute(std::string script);

 private:
  void bind_static_function(module* m,
                            const module::static_function_info& func);

  void bind_class(module* m,
                  const module::implementation_info& info);

  void bind_registered_module(module* m);

  lua_State* L;
};

}  // namespace lua
}  // namespace script
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SCRIPT_LUA_LUA_SCRIPT_ENGINE_H_
