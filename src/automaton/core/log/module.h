#ifndef AUTOMATON_CORE_LOG_MODULE_H_
#define AUTOMATON_CORE_LOG_MODULE_H_

#include "automaton/core/data/schema.h"

namespace automaton {
namespace core {
namespace log {

struct module {
  static constexpr auto name = "log";
  static constexpr auto version = "0.0.1";
  static bool registered;
  static data::schema* get_schema();
};

}  // namespace log
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_LOG_MODULE_H_
