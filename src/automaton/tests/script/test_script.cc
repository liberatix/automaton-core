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

  auto& f = r.get_factory();
  auto id = f.get_schema_id("cryptopp.v0.sha256");
  std::unique_ptr<data::msg> m = f.new_message(id);
  auto obj = r.create(*m.get());

  auto sha256 = dynamic_cast<crypto::SHA256_cryptopp*>(obj);

  std::cout << r.to_string() << std::endl;
}

}  // namespace core
}  // namespace automaton
