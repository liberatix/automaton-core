#include "crypto/module.h"
#include "script/registry.h"

namespace automaton {
namespace core {
namespace crypto {

static bool register_self() {
  script::registry::get().bind<module>();
  return true;
}

bool module::registered = register_self();

}  // namespace crypto
}  // namespace core
}  // namespace automaton
