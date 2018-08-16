#include "automaton/core/script/engine.h"

#include <iomanip>

#include "automaton/core/io/io.h"
#include "automaton/core/data/schema.h"

using automaton::core::data::schema;

namespace automaton {
namespace core {
namespace script {

engine::engine() {
  LOG(DEBUG) << "Creating script engine";
  open_libraries();
  // luaL_openlibs(L);
  // bind_registered_modules();
}

engine::~engine() {
  LOG(DEBUG) << "Destroying script engine";
}

void engine::bind_io() {
  set_function("hex", [](const std::string& s) {
    return io::bin2hex(s);
  });
  set_function("bin", [](const std::string& s) {
    return io::hex2bin(s);
  });
}

void engine::bind_log() {
}

void engine::bind_network() {
}

void engine::bind_state() {
}

}  // namespace script
}  // namespace core
}  // namespace automaton
