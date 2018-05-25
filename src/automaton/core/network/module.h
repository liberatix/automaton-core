#ifndef AUTOMATON_CORE_NETWORK_MODULE_H__
#define AUTOMATON_CORE_NETWORK_MODULE_H__

#include "automaton/core/data/schema.h"

namespace automaton {
namespace core {
namespace network {

struct module {
  static constexpr auto name = "network";
  static constexpr auto version = "0.0.1";
  static bool registered;
  static data::schema* get_schema();
};

}  // namespace network
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_NETWORK_MODULE_H__
