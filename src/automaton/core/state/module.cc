#include "automaton/core/state/module.h"
#include "automaton/core/script/registry.h"

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
