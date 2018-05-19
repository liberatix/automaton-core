#include <boost/di.hpp>
#include "gtest/gtest.h"

namespace di = boost::di;

// FORWARD BINDINGS
namespace forward_bindings {

class interface;
class implementation;

auto configuration = [] {
  // <<binding using fwd declarations, no checking whether types are related
  return di::make_injector(di::bind<interface>().to<implementation>());
};

/*<<binding using fwd declarations, no checking whether types are related*/
class interface {
 public:
  virtual ~interface() noexcept = default;
  virtual void dummy() = 0;
};

class implementation : public interface {
 public:
  void dummy() override {}
};

TEST(test_di, forward_bindings) {
  /*<<make injector configuration>>*/
  auto injector = configuration();
  ASSERT_TRUE(dynamic_cast<implementation*>(injector.create<std::unique_ptr<interface>>().get()));
}

}  // namespace forward_bindings

// DYNAMIC BINDINGS
namespace dynamic_bindings {
enum eid { e1 = 1, e2 = 2 };
struct interface {
  virtual ~interface() noexcept = default;
};
struct implementation1 : interface {};
struct implementation2 : interface {};

auto dynamic_bindings = [](const eid& id) {
  return di::make_injector(
      /*<<bind `interface` to lazy lambda expression>>*/
      di::bind<interface>().to([&](const auto& injector) -> std::shared_ptr<interface> {
        switch (id) {
          default:
            return nullptr;
          case e1:
            return injector.template create<std::shared_ptr<implementation1>>();
          case e2:
            return injector.template create<std::shared_ptr<implementation2>>();
        }

        return nullptr;
      }));
};

TEST(test_di, dynamic_bindings) {
  auto id = e1;

  /*<<create interface with `id = e1`>>*/
  auto injector = di::make_injector(dynamic_bindings(id));
  ASSERT_TRUE(dynamic_cast<implementation1*>(injector.create<std::shared_ptr<interface>>().get()));

  id = e2;
  /*<<create interface with `id = e2`>>*/
  ASSERT_TRUE(dynamic_cast<implementation2*>(injector.create<std::shared_ptr<interface>>().get()));
  (void)id;
}

}  // namespace dynamic_bindings
