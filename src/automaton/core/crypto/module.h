#ifndef AUTOMATON_CORE_CRYPTO_MODULE_H__
#define AUTOMATON_CORE_CRYPTO_MODULE_H__

#include "automaton/core/data/schema.h"
#include "automaton/core/script/registry.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"

namespace automaton {
namespace core {
namespace crypto {

class module: public script::module {
 public:
  static module& instance() {
    static module inst;
    return inst;
  }

  data::schema* schema() const;

 private:
  module() : script::module("crypto", "0.0.1.a") {
    add_concept("hash");
    add_concept("dsig");
    add_concept("rand");
  }
};

}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_CRYPTO_MODULE_H__
