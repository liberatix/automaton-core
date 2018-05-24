#ifndef AUTOMATON_CORE_IO_MODULE_H__
#define AUTOMATON_CORE_IO_MODULE_H__

#include "automaton/core/data/schema.h"

namespace automaton {
namespace core {
namespace io {

struct module {
  static constexpr auto name = "io";
  static constexpr auto version = "0.0.1";
  static bool registered;
  static data::schema* get_schema();
};

}  // namespace io
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_IO_MODULE_H__
