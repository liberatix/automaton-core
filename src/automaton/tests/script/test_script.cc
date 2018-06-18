#include "automaton/core/crypto/module.h"

#include "automaton/core/crypto/cryptopp/module.h"
#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/crypto/ed25519_orlp/module.h"
#include "automaton/core/data/module.h"
#include "automaton/core/data/protobuf/module.h"
#include "automaton/core/io/module.h"
#include "automaton/core/network/module.h"
#include "automaton/core/log/module.h"
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

}  // namespace core
}  // namespace automaton
