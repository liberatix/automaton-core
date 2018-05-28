#ifndef AUTOMATON_CORE_STATE_MODULE_H__
#define AUTOMATON_CORE_STATE_MODULE_H__

#include "automaton/core/data/schema.h"
#include "automaton/core/script/registry.h"

namespace automaton {
namespace core {
namespace state {

class module: public script::module {
 public:
  static module& instance() {
    static module inst;
    return inst;
  }

  data::schema* schema() const;

 private:
  module() : script::module("state", "0.0.1.a") {}
};

}  // namespace state
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_STATE_MODULE_H_
