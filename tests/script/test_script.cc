#include "data/msg.h"
#include "data/factory.h"
#include "data/schema.h"
#include "script/registry.h"
#include "gtest/gtest.h"

#include "crypto/module.h"
#include "data/module.h"
#include "io/module.h"
#include "network/module.h"
#include "state/module.h"

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
