#ifndef AUTOMATON_CORE_IO_MODULE_H__
#define AUTOMATON_CORE_IO_MODULE_H__

#include "automaton/core/data/schema.h"
#include "automaton/core/io/io.h"
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

  static common::status tohex(const data::msg& input, data::msg * output) {
    // TODO(asen): Check message schema IDs for type safety
    output->set_blob(1, string_to_hex(input.get_blob(1)));
    return common::status(common::status::OK);
  }

 private:
  module() : script::module("io", "0.0.1.a") {
    add_function("tohex", &tohex);
  }
};

}  // namespace io
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_IO_MODULE_H__
