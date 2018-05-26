#include "automaton/core/log/module.h"

#include "automaton/core/io/io.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"

using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::io::get_file_contents;

namespace automaton {
namespace core {
namespace log {

data::schema* module::schema() const {
  static protobuf_schema* schema_ = nullptr;
  if (schema_ == nullptr) {
    schema_ = new protobuf_schema(get_file_contents("automaton/core/log/module.proto"));
  }
  return schema_;
}

}  // namespace log
}  // namespace core
}  // namespace automaton
