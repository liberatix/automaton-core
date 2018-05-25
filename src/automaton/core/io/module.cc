#include "automaton/core/io/module.h"
#include "automaton/core/script/registry.h"

namespace automaton {
namespace core {
namespace io {

static bool register_self() {
  script::registry::get().bind<module>();
  return true;
}

bool module::registered = register_self();

data::schema* module::get_schema() {
  return nullptr;
}

}  // namespace io
}  // namespace core
}  // namespace automaton
