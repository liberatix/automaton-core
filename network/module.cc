#include "network/module.h"
#include "script/registry.h"

namespace automaton {
namespace core {
namespace network {

static bool register_self() {
  script::registry::get().bind<module>();
  return true;
}

bool module::registered = register_self();

}  // namespace network
}  // namespace core
}  // namespace automaton
