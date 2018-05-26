#include "automaton/core/crypto/cryptopp/module.h"

#include <iostream>
#include <string>

#include "automaton/core/io/io.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"

using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::io::get_file_contents;

namespace automaton {
namespace core {
namespace crypto {
namespace cryptopp {

data::schema* module::schema() const {
  static protobuf_schema* schema_ = nullptr;
  if (schema_ == nullptr) {
    schema_ = new protobuf_schema(
        get_file_contents("automaton/core/crypto/cryptopp/module.proto"));
  }
  return schema_;
}

}  // namespace cryptopp
}  // namespace crypto
}  // namespace core
}  // namespace automaton
