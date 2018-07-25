#ifndef AUTOMATON_CORE_LOG_MODULE_H_
#define AUTOMATON_CORE_LOG_MODULE_H_

#include "automaton/core/data/schema.h"
#include "automaton/core/log/log.h"
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

  static common::status info(const data::msg& input, data::msg * output) {
    LOG(INFO) << input.get_blob(1);
    return common::status(common::status::OK);
  }

  static common::status debug(const data::msg& input, data::msg * output) {
    LOG(DEBUG) << input.get_blob(1);
    return common::status(common::status::OK);
  }

  static common::status warning(const data::msg& input, data::msg * output) {
    LOG(WARNING) << input.get_blob(1);
    return common::status(common::status::OK);
  }

  static common::status error(const data::msg& input, data::msg * output) {
    LOG(ERROR) << input.get_blob(1);
    return common::status(common::status::OK);
  }

  static common::status fatal(const data::msg& input, data::msg * output) {
    LOG(FATAL) << input.get_blob(1);
    return common::status(common::status::OK);
  }

 private:
  module() : script::module("log", "0.0.1.a") {
    add_function("info", info);
    add_function("debug", debug);
    add_function("warning", warning);
    add_function("error", error);
    add_function("fatal", fatal);
  }
};

}  // namespace log
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_LOG_MODULE_H_
