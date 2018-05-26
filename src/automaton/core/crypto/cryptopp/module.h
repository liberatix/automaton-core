#ifndef AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_
#define AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_

#include "automaton/core/data/schema.h"
#include "automaton/core/script/registry.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"

namespace automaton {
namespace core {
namespace crypto {
namespace cryptopp {

class module: public script::module {
 public:
  static module& instance() {
    static module inst;
    return inst;
  }

  data::schema* schema() const;

 private:
  module() : script::module("crypto.cryptopp", "0.0.1.a") {
    add_dependency("crypto", 0);
  }
};

}  // namespace cryptopp
}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_
