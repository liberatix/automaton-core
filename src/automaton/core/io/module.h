#ifndef AUTOMATON_CORE_IO_MODULE_H__
#define AUTOMATON_CORE_IO_MODULE_H__

#include "automaton/core/data/schema.h"
#include "automaton/core/script/registry.h"

namespace automaton {
namespace core {
namespace io {

class module: public script::module {
 public:
  static module& instance() {
    static module inst;
    return inst;
  }

  data::schema* schema() const;

 private:
  module() : script::module("io", "0.0.1.a") {}
};

}  // namespace io
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_IO_MODULE_H__
