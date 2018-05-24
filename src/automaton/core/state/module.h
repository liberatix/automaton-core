#ifndef AUTOMATON_CORE_STATE_MODULE_H__
#define AUTOMATON_CORE_STATE_MODULE_H__

#include "automaton/core/script/registry.h"

namespace automaton {
namespace core {
namespace state {

struct module {
  static constexpr auto name = "state";
  static constexpr auto version = "0.0.1";
  static bool registered;
};

}  // namespace state
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_STATE_MODULE_H_
