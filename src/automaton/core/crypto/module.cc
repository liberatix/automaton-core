#include "automaton/core/crypto/module.h"
#include "automaton/core/io/io.h"
#include "automaton/core/script/registry.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"

using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::io::get_file_contents;

namespace automaton {
namespace core {
namespace crypto {

bool module::registered = [] {
  script::registry::get().bind<module>();
  return true;
}();

data::schema* module::get_schema() {
  static protobuf_schema* schema_ = nullptr;
  if (schema_ == nullptr) {
    schema_ = new protobuf_schema(get_file_contents("automaton/core/crypto/schema.proto"));
  }
  return schema_;
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
