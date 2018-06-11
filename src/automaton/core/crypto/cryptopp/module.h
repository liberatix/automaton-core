#ifndef AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_
#define AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_

#include "automaton/core/common/obj.h"
#include "automaton/core/crypto/cryptopp/Keccak_256_cryptopp.h"
#include "automaton/core/crypto/cryptopp/RIPEMD160_cryptopp.h"
#include "automaton/core/crypto/cryptopp/secp256k1_cryptopp.h"
#include "automaton/core/crypto/cryptopp/secure_random_cryptopp.h"
#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/crypto/cryptopp/SHA3_256_cryptopp.h"
#include "automaton/core/crypto/cryptopp/SHA512_cryptopp.h"
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

  static common::obj* create_sha3(const data::msg& m) {
    return new SHA3_256_cryptopp();
  }

  static common::obj* create_sha512(const data::msg& m) {
    return new SHA512_cryptopp();
  }

  static common::obj* create_keccak256(const data::msg& m) {
    return new Keccak_256_cryptopp();
  }

  static common::obj* create_ripemd160(const data::msg& m) {
    return new RIPEMD160_cryptopp();
  }

  static common::obj* create_random(const data::msg& m) {
    return new secure_random_cryptopp();
  }

  static common::obj* create_secp256k1(const data::msg& m) {
    return new secp256k1_cryptopp();
  }

 private:
  module() : script::module("cryptopp", "0.0.1.a") {
    add_dependency("crypto", 0);

    add_implementation("keccak256", &create_keccak256);
    add_implementation("ripemd160", &create_ripemd160);
    add_implementation("sha256", &create_sha256);
    add_implementation("sha3", &create_sha3);
    add_implementation("sha512", &create_sha512);

    add_implementation("random", &create_random);

    add_implementation("secp256k1", &create_secp256k1);
  }
};

}  // namespace cryptopp
}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_
