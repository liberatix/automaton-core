#include "automaton/core/crypto/module.h"

#include "automaton/core/crypto/cryptopp/module.h"
#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/crypto/ed25519_orlp/module.h"
#include "automaton/core/data/module.h"
#include "automaton/core/data/protobuf/module.h"
#include "automaton/core/io/module.h"
#include "automaton/core/log/module.h"
#include "automaton/core/network/module.h"
#include "automaton/core/script/registry.h"
#include "automaton/core/state/module.h"
#include "gtest/gtest.h"

namespace automaton {
namespace core {

class test_script : public ::testing::Test {
 protected:
  // You can define per-test set-up and tear-down logic as usual.
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

common::obj* create_by_name(std::string name) {
  auto& r = script::registry::instance();
  auto& f = r.get_factory();
  auto id = f.get_schema_id(name);
  std::unique_ptr<data::msg> m = f.new_message_by_id(id);
  return r.create(*m.get());
}

TEST_F(test_script, module_registration) {
  auto& r = script::registry::instance();

  // import core module interfaces.
  r.import<crypto::module>();
  r.import<data::module>();
  r.import<io::module>();
  r.import<log::module>();
  r.import<network::module>();
  r.import<state::module>();

  // import core module implementations.
  r.import<crypto::cryptopp::module>();
  r.import<crypto::ed25519_orlp::module>();
  r.import<data::protobuf::module>();

  // Test instantiation of all hash objects.
  auto keccak256 = dynamic_cast<crypto::SHA256_cryptopp*>(create_by_name("cryptopp.v0.keccak256"));
  auto ripemd160 = dynamic_cast<crypto::SHA256_cryptopp*>(create_by_name("cryptopp.v0.ripemd160"));
  auto sha256 = dynamic_cast<crypto::SHA256_cryptopp*>(create_by_name("cryptopp.v0.sha256"));
  auto sha3 = dynamic_cast<crypto::SHA256_cryptopp*>(create_by_name("cryptopp.v0.sha3"));
  auto sha512 = dynamic_cast<crypto::SHA256_cryptopp*>(create_by_name("cryptopp.v0.sha512"));

  // Test instantiation of random object.
  auto random = dynamic_cast<crypto::SHA256_cryptopp*>(create_by_name("cryptopp.v0.random"));

  // Test instantiation of digital signatures object.
  auto secp256k1 = dynamic_cast<crypto::SHA256_cryptopp*>(create_by_name("cryptopp.v0.secp256k1"));
  auto ed25519 = dynamic_cast<crypto::SHA256_cryptopp*>(create_by_name("ed25519_orlp.v0.ed25519"));

  std::cout << r.to_string() << std::endl;
}

}  // namespace core
}  // namespace automaton
