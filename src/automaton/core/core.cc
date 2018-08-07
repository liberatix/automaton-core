#include <iostream>
#include <regex>
#include <string>

#include <json.hpp>

#include "automaton/core/cli/cli.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "automaton/core/io/io.h"
#include "automaton/core/log/log.h"
#include "automaton/core/script/lua/lua_script_engine.h"
#include "automaton/core/smartproto/node.h"

using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::data::schema;
using automaton::core::io::get_file_contents;
using automaton::core::script::lua::lua_script_engine;
using automaton::core::smartproto::node;

using json = nlohmann::json;

using std::make_unique;
using std::unique_ptr;

void string_replace(std::string* str,
                    const std::string& oldStr,
                    const std::string& newStr) {
  std::string::size_type pos = 0u;
  while ((pos = str->find(oldStr, pos)) != std::string::npos) {
     str->replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}

static const char* automaton_ascii_logo_cstr =
  "\n\x1b[40m\x1b[1m"
  "                                                                    " "\x1b[0m\n\x1b[40m\x1b[1m"
  "                                                                    " "\x1b[0m\n\x1b[40m\x1b[1m"
  "    @197m█▀▀▀█ @39m█ █ █ @11m▀▀█▀▀ @129m█▀▀▀█ @47m█▀█▀█ @9m█▀▀▀█ @27m▀▀█▀▀ @154m█▀▀▀█ @13m█▀█ █           " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "    @197m█▀▀▀█ @39m█ ▀ █ @11m█ █ █ @129m█ ▀ █ @47m█ ▀ █ @9m█▀▀▀█ @27m█ █ █ @154m█ ▀ █ @13m█ █ █  @15mCORE     " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "    @197m▀ ▀ ▀ @39m▀▀▀▀▀ @11m▀ ▀ ▀ @129m▀▀▀▀▀ @47m▀ ▀ ▀ @9m▀ ▀ ▀ @27m▀ ▀ ▀ @154m▀▀▀▀▀ @13m▀ ▀▀▀  @15mv0.0.1   " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "                                                                    " "\x1b[0m\n@0m"
  "▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀" "\x1b[0m\n"
  "  @7mThese are common Automaton commands used in various situations:\n"
  "\n"
  "     \x1b[1m@15m.modules    \x1b[0m@7mShow list of registered modules\n"
  "     \x1b[1m@15m.protos     \x1b[0m@7mShow list of registered smart protocol definitions\n"
  "     \x1b[1m@15m.nodes      \x1b[0m@7mShow list of node instances running on this client\n"
  "     \x1b[1m@15m.launch     \x1b[0m@7mLaunch a smart protocol node instance from a definiition\n"
  "     \x1b[1m@15m.use        \x1b[0m@7mSet the current smart protocol node\n"
  "     \x1b[1m@15m.msg        \x1b[0m@7mConstruct and send a message to the current smart protocol\n\n"; // NOLINT

int main(int argc, char* argv[]) {
  std::string automaton_ascii_logo(automaton_ascii_logo_cstr);
  string_replace(&automaton_ascii_logo, "@", "\x1b[38;5;");

  lua_script_engine engine;
  engine.bind_core();
  sol::state_view& lua = engine.get_sol();
/*
  // Bind smartproto::node constructor
  lua.set_function("SPNode", [](const char* schema_file_name, const char* proto_file_name) {
    auto schema_contents = get_file_contents(schema_file_name);
    auto script_contents = get_file_contents(proto_file_name);
    unique_ptr<schema> pb_schema(new protobuf_schema(schema_contents));
    return new node(std::move(pb_schema), script_contents);
  });
*/

  // Bind smartproto::node class
  lua.new_usertype<node>("SPNode",
    sol::call_constructor,
    [](const char* schema_file_name, const char* proto_file_name) {
      auto schema_contents = get_file_contents(schema_file_name);
      auto script_contents = get_file_contents(proto_file_name);
      unique_ptr<schema> pb_schema(new protobuf_schema(schema_contents));
      return new node(std::move(pb_schema), script_contents);
    }
  );  // NOLINT

  lua.script(
      R"(
      function BCNode()
        return SPNode(
          "automaton/examples/smartproto/blockchain/blockchain.proto",
          "automaton/examples/smartproto/blockchain/blockchain.lua"
        )
      end
      )");

  automaton::core::cli::cli cli;
  std::cout << automaton_ascii_logo;
  while (1) {
    // auto cmd = cli.input("\x1b[38;5;15m\x1b[1m|A|\x1b[0m ");
    auto input = cli.input("\x1b[38;5;15m\x1b[1m 🄰 \x1b[0m ");
    if (input == nullptr) {
      printf("\n");
      break;
    }

    std::string cmd{input};
    cli.history_add(cmd.c_str());

    sol::protected_function_result pfr = lua.safe_script(cmd, &sol::script_pass_on_error);
    std::string output = pfr;
    std::cout << output << std::endl;
  }
  return 0;
}
