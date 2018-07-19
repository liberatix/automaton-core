#include "automaton/core/crypto/cryptopp/module.h"
#include "automaton/core/crypto/ed25519_orlp/module.h"
#include "automaton/core/crypto/module.h"
#include "automaton/core/data/module.h"
#include "automaton/core/data/protobuf/module.h"
#include "automaton/core/io/module.h"
#include "automaton/core/log/module.h"
#include "automaton/core/network/module.h"
#include "automaton/core/script/lua/lua_script_engine.h"
#include "automaton/core/script/registry.h"
#include "automaton/core/state/module.h"

#include "lua.hpp"

using automaton::core::common::status;
using automaton::core::data::msg;
using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::data::schema;
using automaton::core::io::get_file_contents;
using automaton::core::script::lua::lua_script_engine;
using automaton::core::script::module;

class test_module: public module {
 public:
  static test_module& instance() {
    static test_module inst;
    return inst;
  }

  class schema* schema() const {
    static protobuf_schema* schema_ = nullptr;
    if (schema_ == nullptr) {
      auto module_schema = get_file_contents("automaton/examples/script/script_demo_module.proto");
      schema_ = new protobuf_schema(module_schema);
    }
    return schema_;
  }

  static status divide(const msg& input, msg * output) {
    LOG(INFO) << "divide(" << input.get_message_type() << "," << output->get_message_type() << ")";
    int32_t x = input.get_int32(1);
    int32_t y = input.get_int32(2);
    if (y == 0) {
      return status::failed_precondition("Can't divide by 0!");
    }
    output->set_int32(1, x / y);
    return status::ok();
  }

  static status add(const msg& input, msg * output) {
    int32_t x = input.get_int32(1);
    int32_t y = input.get_int32(2);
    output->set_int32(1, x + y);
    return status::ok();
  }

  static status log(const msg& input, msg * output) {
    LOG(INFO) << "LUA print: " << input.get_blob(1);
    return status::ok();
  }

 private:
  test_module() : module("test", "0.0.1.a") {
    add_function("add", &add);
    add_function("divide", &divide);
    add_function("log", &log);
  }
};

void register_modules() {
  auto& r = automaton::core::script::registry::instance();

  // Import test module.
  r.import<test_module>();

  // import core module interfaces.
  // r.import<automaton::core::crypto::module>();
  // r.import<automaton::core::data::module>();
  // r.import<automaton::core::io::module>();
  // r.import<automaton::core::log::module>();
  // r.import<automaton::core::network::module>();
  // r.import<automaton::core::state::module>();

  // import core module implementations.
  // r.import<automaton::core::crypto::cryptopp::module>();
  // r.import<automaton::core::crypto::ed25519_orlp::module>();
  // r.import<automaton::core::data::protobuf::module>();

  // std::cout << r.to_string() << std::endl;
}

int main() {
  register_modules();

  lua_script_engine engine;

  // Should generate an error because of the extra arguments (expecting 2, getting 3).
  auto r = engine.execute("add(1,2,3)");
  if (r.code != status::OK) {
    LOG(ERROR) << "LUA ERROR: " << r.msg;
  }

  // Should generate an error because of division by zero.
  r = engine.execute("divide(5,0)");
  if (r.code != status::OK) {
    LOG(ERROR) << "LUA ERROR: " << r.msg;
  }

  r = engine.execute("log('HELLO!')");
  if (r.code != status::OK) {
    LOG(ERROR) << "LUA ERROR: " << r.msg;
  }

  r = engine.execute(get_file_contents("automaton/examples/script/script_demo.lua"));
  if (r.code != status::OK) {
    LOG(ERROR) << "LUA ERROR: " << r.msg;
  }

  return 0;
}
