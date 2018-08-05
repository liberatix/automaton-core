#ifndef AUTOMATON_CORE_CLI_MODULE_H_
#define AUTOMATON_CORE_CLI_MODULE_H_

#include "automaton/core/data/schema.h"
#include "automaton/core/script/registry.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"

namespace automaton {
namespace core {
namespace cli {

class module: public script::module {
 public:
  static module& instance() {
    static module inst;
    return inst;
  }

  data::schema* schema() const;

 private:
  module() : script::module("cli", "0.0.1.a") {}
};

}  // namespace cli
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_CLI_MODULE_H_
