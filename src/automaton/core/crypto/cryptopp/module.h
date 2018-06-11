#ifndef AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_
#define AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_

#include "automaton/core/common/obj.h"
#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "automaton/core/data/schema.h"
#include "automaton/core/script/registry.h"

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

  static common::obj* create_sha256(const data::msg& m) {
    return new SHA256_cryptopp();
  }

 private:
  module() : script::module("cryptopp", "0.0.1.a") {
    add_dependency("crypto", 0);

    add_implementation("keccak256", nullptr);
    add_implementation("ripemd160", nullptr);
    add_implementation("sha256", &create_sha256);
    add_implementation("sha3", nullptr);
    add_implementation("sha512", nullptr);

    add_implementation("random", nullptr);

    add_implementation("secp256k1", nullptr);
  }
};

}  // namespace cryptopp
}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_
