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

  static common::status wrap_bin2hex(const data::msg& input, data::msg * output) {
    // TODO(asen): Check message schema IDs for type safety
    output->set_blob(1, bin2hex(input.get_blob(1)));
    return common::status::ok();
  }

  static common::status wrap_hex2bin(const data::msg& input, data::msg * output) {
    // TODO(asen): Check message schema IDs for type safety
    output->set_blob(1, hex2bin(input.get_blob(1)));
    return common::status::ok();
  }

 private:
  module() : script::module("io", "0.0.1.a") {
    add_function("hex", &wrap_bin2hex);
    add_function("hex2bin", &wrap_hex2bin);
  }
};

}  // namespace io
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_IO_MODULE_H__
