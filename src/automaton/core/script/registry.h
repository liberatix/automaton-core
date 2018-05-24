#ifndef AUTOMATON_CORE_SCRIPT_REGISTRY_H_
#define AUTOMATON_CORE_SCRIPT_REGISTRY_H_

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "automaton/core/data/factory.h"
#include "automaton/core/data/schema.h"

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
    // TODO(asen): Get schema import to work.
    auto module_schema = M::get_schema();
    if (module_schema != nullptr) {
      factory_->import_schema(module_schema, M::name, M::name);
    }
    modules_[std::string(M::name) + ":" + std::string(M::version)] =
      {M::name, M::version, module_schema};
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
    data::schema* schema;
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
