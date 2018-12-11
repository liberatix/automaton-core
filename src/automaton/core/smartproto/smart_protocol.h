#ifndef AUTOMATON_CORE_SMARTPROTO_SMART_PROTOCOL_H_
#define AUTOMATON_CORE_SMARTPROTO_SMART_PROTOCOL_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "automaton/core/data/protobuf/protobuf_schema.h"

namespace automaton {
namespace core {
namespace smartproto {

class smart_protocol {
 public:
  struct cmd {
    std::string name;
    std::string input_type;
    std::string output_type;

    cmd(std::string nm, std::string input, std::string output):name(nm), input_type(input), output_type(output) {}
  };

  smart_protocol();
  bool load(std::string path);

  std::unordered_map<std::string, std::string> get_proto_definitions();

 private:
  std::string id;
  uint32_t update_time_slice;

  std::unordered_map<std::string, std::string> proto_defs;
  std::vector<data::protobuf::protobuf_schema::schema*> schemas;
  std::vector<std::string> lua_scripts;
  std::unordered_map<uint32_t, uint32_t> wire_to_factory;
  std::unordered_map<uint32_t, uint32_t> factory_to_wire;
  std::vector<std::string> wire_msgs;
  std::vector<cmd> commands;
};

}  // namespace smartproto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SMARTPROTO_SMART_PROTOCOL_H_
