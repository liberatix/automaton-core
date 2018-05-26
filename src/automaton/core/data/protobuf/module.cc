#include "automaton/core/data/protobuf/module.h"

#include "automaton/core/io/io.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"

using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::io::get_file_contents;

namespace automaton {
namespace core {
namespace data {
namespace protobuf {

data::schema* module::schema() const {
  static protobuf_schema* schema_ = nullptr;
  if (schema_ == nullptr) {
    schema_ = new protobuf_schema(get_file_contents("automaton/core/data/module.proto"));
  }
  return schema_;
}

}  // namespace protobuf
}  // namespace data
}  // namespace core
}  // namespace automaton
