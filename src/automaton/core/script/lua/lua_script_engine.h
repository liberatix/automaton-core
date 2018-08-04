#ifndef AUTOMATON_CORE_SCRIPT_LUA_LUA_SCRIPT_ENGINE_H_
#define AUTOMATON_CORE_SCRIPT_LUA_LUA_SCRIPT_ENGINE_H_

#include <memory>
#include <string>

#include "automaton/core/crypto/hash_transformation.h"
#include "automaton/core/script/registry.h"

#include "sol.hpp"

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

  void bind_core() {
    bind_crypto();
    bind_data();
    bind_io();
    bind_log();
    bind_network();
    bind_state();
  }

  void bind_crypto();

  void bind_data();

  void bind_io();

  void bind_log();

  void bind_network();

  void bind_state();

  /**
  */
  void bind_registered_modules();

  /**
  */
  common::status execute(std::string script);

  auto get_lua_state() { return L; }

 private:
  void bind_static_function(module* m,
                            const module::static_function_info& func);

  void bind_class(module* m,
                  const module::implementation_info& info);

  void bind_registered_module(module* m);

  lua_State* L;
  sol::state lua;

  // Crypto hash functions
  std::unique_ptr<crypto::hash_transformation> ripemd160;
  std::unique_ptr<crypto::hash_transformation> sha512;
  std::unique_ptr<crypto::hash_transformation> sha256;
  std::unique_ptr<crypto::hash_transformation> sha3;
  std::unique_ptr<crypto::hash_transformation> keccak256;
};

}  // namespace lua
}  // namespace script
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SCRIPT_LUA_LUA_SCRIPT_ENGINE_H_
