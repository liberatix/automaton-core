#ifndef AUTOMATON_CORE_SCRIPTABLE_SCRIPTABLE_H__
#define AUTOMATON_CORE_SCRIPTABLE_SCRIPTABLE_H__

#include <boost/core/demangle.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include <string>
#include <iostream>
#include <unordered_map>

#include "data/factory.h"
#include "data/msg.h"
#include "data/schema.h"

namespace automaton {
namespace core {
namespace script {

/**
  Registry for scriptable modules.
*/

class registry {
 public:
  registry(registry&) = delete;
  registry(const registry&) = delete;
  struct module_info {
    std::string version;
  };

  /**
    Binds module M to the script::registry.
  */
  template <typename M>
  void bind() {
    modules[M::name] = {M::version};
  }

  std::string to_string();

  static registry& get() {
    static registry * instance = nullptr;
    if (instance == nullptr) {
      instance = new registry();
    }
    return *instance;
  }

 private:
  registry() {}

  std::unordered_map<std::string, module_info> modules;
};

}  // namespace script
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SCRIPTABLE_SCRIPTABLE_H__
