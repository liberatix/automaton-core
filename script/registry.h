#ifndef AUTOMATON_CORE_SCRIPTABLE_SCRIPTABLE_H__
#define AUTOMATON_CORE_SCRIPTABLE_SCRIPTABLE_H__

#include <string>
#include <unordered_map>

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
