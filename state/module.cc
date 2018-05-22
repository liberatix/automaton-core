#include "state/module.h"
#include "script/registry.h"

namespace automaton {
namespace core {
namespace state {

static bool register_self() {
  script::registry::get().bind<module>();
  return true;
}

bool module::registered = register_self();

}  // namespace state
}  // namespace core
}  // namespace automaton
