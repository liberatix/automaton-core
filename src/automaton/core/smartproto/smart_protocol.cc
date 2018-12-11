#include "automaton/core/smartproto/smart_protocol.h"

#include <fstream>
#include <string>

#include <json.hpp>

#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "automaton/core/io/io.h"

using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::io::get_file_contents;

namespace automaton {
namespace core {
namespace smartproto {

smart_protocol::smart_protocol() {}

bool smart_protocol::load(std::string path) {
  std::ifstream i(path + "config.json");
  if (!i.is_open()) {
    LOG(ERROR) << "Error while opening " << path << "config.json";
    return false;
  } else {
    nlohmann::json j;
    i >> j;
    i.close();
    update_time_slice = j["update_time_slice"];
    std::vector<std::string> schemas_filenames = j["schemas"];
    std::vector<std::string> lua_scripts_filenames = j["lua_scripts"];
    std::vector<std::string> wm = j["wire_msgs"];
    wire_msgs = wm;
    for (auto cmd : j["commands"]) {
      commands.push_back({cmd[0], cmd[1], cmd[2]});
    }

    for (uint32_t i = 0; i < schemas_filenames.size(); ++i) {
      std::string file_content = get_file_contents((path + schemas_filenames[i]).c_str());
      proto_defs[schemas_filenames[i]] = file_content;
      schemas.push_back(new protobuf_schema(file_content));
    }

    for (uint32_t i = 0; i < lua_scripts_filenames.size(); ++i) {
      lua_scripts.push_back(get_file_contents((path + lua_scripts_filenames[i]).c_str()));
    }
  }
  return true;
}

std::unordered_map<std::string, std::string> smart_protocol::get_proto_definitions() {
  return proto_defs;
}

}  // namespace smartproto
}  // namespace core
}  // namespace automaton
