#include "automaton/core/log/module.h"
#include "automaton/core/script/registry.h"

namespace automaton {
namespace core {
namespace log {

static bool register_self() {
  script::registry::get().bind<module>();
  return true;
}

bool module::registered = register_self();

}  // namespace log
}  // namespace core
}  // namespace automaton
