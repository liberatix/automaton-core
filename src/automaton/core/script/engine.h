#ifndef AUTOMATON_CORE_SCRIPT_ENGINE_H_
#define AUTOMATON_CORE_SCRIPT_ENGINE_H_

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "automaton/core/crypto/hash_transformation.h"

#include "sol.hpp"

namespace automaton {
namespace core {
namespace script {

/**
  Lua script engine wrapper and Autoamaton's module bridge.
*/
class engine : public sol::state {
 public:
  engine();
  ~engine();

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

 private:
  // Crypto hash functions
  std::unique_ptr<crypto::hash_transformation> ripemd160;
  std::unique_ptr<crypto::hash_transformation> sha512;
  std::unique_ptr<crypto::hash_transformation> sha256;
  std::unique_ptr<crypto::hash_transformation> sha3;
  std::unique_ptr<crypto::hash_transformation> keccak256;
};

}  // namespace script
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SCRIPT_ENGINE_H_