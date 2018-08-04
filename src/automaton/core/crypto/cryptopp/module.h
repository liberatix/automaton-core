#ifndef AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_
#define AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_

#include <memory>

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

  static std::unique_ptr<common::obj> create_sha256(const data::msg& m) {
    return std::unique_ptr<common::obj>(new SHA256_cryptopp());
  }

  static std::unique_ptr<common::obj> create_sha3(const data::msg& m) {
    return std::unique_ptr<common::obj>(new SHA3_256_cryptopp());
  }

  static std::unique_ptr<common::obj> create_sha512(const data::msg& m) {
    return std::unique_ptr<common::obj>(new SHA512_cryptopp());
  }

  static std::unique_ptr<common::obj> create_keccak256(const data::msg& m) {
    return std::unique_ptr<common::obj>(new Keccak_256_cryptopp());
  }

  static std::unique_ptr<common::obj> create_ripemd160(const data::msg& m) {
    return std::unique_ptr<common::obj>(new RIPEMD160_cryptopp());
  }

  static std::unique_ptr<common::obj> create_random(const data::msg& m) {
    return std::unique_ptr<common::obj>(new secure_random_cryptopp());
  }

  static std::unique_ptr<common::obj> create_secp256k1(const data::msg& m) {
    return std::unique_ptr<common::obj>(new secp256k1_cryptopp());
  }

 private:
  module() : script::module("cryptopp", "0.0.1.a") {
    add_dependency("crypto", 0);

    add_implementation("keccak256", {"crypto.v0.hash"}, &create_keccak256);
    add_implementation("ripemd160", {"crypto.v0.hash"}, &create_ripemd160);
    add_implementation("sha256", {"crypto.v0.hash"}, &create_sha256);
    add_implementation("sha3", {"crypto.v0.hash"}, &create_sha3);
    add_implementation("sha512", {"crypto.v0.hash"}, &create_sha512);

    add_implementation("random", {"crypto.v0.rand"}, &create_random);

    add_implementation("secp256k1", {"crypto.v0.dsig"}, &create_secp256k1);
  }
};

}  // namespace cryptopp
}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_CRYPTO_CRYPTOPP_MODULE_H_
