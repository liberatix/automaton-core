#ifndef AUTOMATON_CORE_SCRIPT_REGISTRY_H_
#define AUTOMATON_CORE_SCRIPT_REGISTRY_H_

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "automaton/core/data/factory.h"

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

  /**
    Binds module M to the script::registry.
  */
  template <typename M>
  void bind() {
    modules_[std::string(M::name) + ":" + std::string(M::version)] = {M::name, M::version};
    // TODO(asen): Get schema import to work.
    // factory_->import_schema(M::get_schema(), M::name, M::name);
  }

  /**
    Dumps information about all registered modules, functions, classes and schemas into a string.
  */
  std::string to_string();

  /**
    Gets reference to the registry singleton instance.

    Creates the instance when called for the first time.
  */
  static registry& get();

 private:
  struct function_info {
  };

  struct module_info {
    std::string name;
    std::string version;
    std::vector<std::string> functions;
    std::vector<std::string> classes;
  };

  registry();

  std::unordered_map<std::string, module_info> modules_;
  std::unique_ptr<data::factory> factory_;
};

}  // namespace script
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SCRIPT_REGISTRY_H_
