#include "automaton/core/script/engine.h"

#include <iomanip>

#include "automaton/core/io/io.h"
#include "automaton/core/data/factory.h"
#include "automaton/core/data/msg.h"
#include "automaton/core/data/schema.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"

using automaton::core::data::factory;
using automaton::core::data::msg;
using automaton::core::data::schema;
using automaton::core::data::protobuf::protobuf_factory;

namespace automaton {
namespace core {
namespace script {

engine::engine(factory& data_factory)
    : data_factory(data_factory) {
  // TODO(asen): This is convenient, but dangerous! Get rid of it.
  open_libraries();
}

engine::~engine() {
  collect_garbage();
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

void engine::import_schema(data::schema* msg_schema) {
  data_factory.import_schema(msg_schema, "", "");
  auto schemas_number = data_factory.get_schemas_number();

  // Bind schema messages.
  auto msg_names = msg_schema->get_message_names();
  for (auto msg_name : msg_names) {
    auto msg_id = data_factory.get_schema_id(msg_name);
    set_function(msg_name, [this, msg_name, msg_id]() -> unique_ptr<msg> {
      return data_factory.new_message_by_id(msg_id);
    });
  }
}

}  // namespace script
}  // namespace core
}  // namespace automaton
