#ifndef AUTOMATON_CORE_DATA_PROTOBUF_MODULE_H_
#define AUTOMATON_CORE_DATA_PROTOBUF_MODULE_H_

#include "automaton/core/data/schema.h"
#include "automaton/core/script/registry.h"

namespace automaton {
namespace core {
namespace data {
namespace protobuf {

class module: public script::module {
 public:
  static module& instance() {
    static module inst;
    return inst;
  }

  data::schema* schema() const;

 private:
  module() : script::module("data.protobuf", "0.0.1.a") {
    add_dependency("data", 0);
  }
};

}  // namespace protobuf
}  // namespace data
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_DATA_PROTOBUF_MODULE_H_
