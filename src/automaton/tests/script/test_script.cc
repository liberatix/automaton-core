#include "automaton/core/data/msg.h"
#include "automaton/core/data/factory.h"
#include "automaton/core/data/schema.h"
#include "automaton/core/script/registry.h"
#include "gtest/gtest.h"

#include "automaton/core/crypto/module.h"
#include "automaton/core/data/module.h"
#include "automaton/core/io/module.h"
#include "automaton/core/network/module.h"
#include "automaton/core/state/module.h"

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
  auto& r = script::registry::get();
  ASSERT_TRUE(crypto::module::registered);
  ASSERT_TRUE(data::module::registered);
  ASSERT_TRUE(io::module::registered);
  ASSERT_TRUE(network::module::registered);
  ASSERT_TRUE(state::module::registered);
  std::cout << r.to_string() << std::endl;
}

}  // namespace core
}  // namespace automaton
