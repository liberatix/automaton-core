#ifndef AUTOMATON_CORE_LOG_MODULE_H_
#define AUTOMATON_CORE_LOG_MODULE_H_

#include "automaton/core/data/schema.h"
#include "automaton/core/script/registry.h"

namespace automaton {
namespace core {
namespace log {

class module: public script::module {
 public:
  static module& instance() {
    static module inst;
    return inst;
  }

  data::schema* schema() const;

 private:
  module() : script::module("log", "0.0.1.a") {}
};

}  // namespace log
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_LOG_MODULE_H_
