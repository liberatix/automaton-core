#include "automaton/core/common/module.h"

#include <iostream>
#include <string>

#include "automaton/core/io/io.h"

namespace automaton {
namespace core {
namespace common {

data::schema* module::schema() const {
  static protobuf_schema* schema_ = nullptr;
  if (schema_ == nullptr) {
    schema_ = new protobuf_schema(get_file_contents("automaton/core/common/module.proto"));
  }
  return schema_;
}

}  // namespace common
}  // namespace core
}  // namespace automaton
