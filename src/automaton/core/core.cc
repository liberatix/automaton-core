#include <iostream>
#include <regex>
#include <string>

#include <json.hpp>

#include "automaton/core/network/tcp_implementation.h"
#include "automaton/core/cli/cli.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "automaton/core/io/io.h"
#include "automaton/core/network/simulated_connection.h"
#include "automaton/core/script/engine.h"
#include "automaton/core/smartproto/node.h"

using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::data::schema;
using automaton::core::io::get_file_contents;
using automaton::core::script::engine;
using automaton::core::smartproto::node;

using json = nlohmann::json;

using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;

void string_replace(string* str,
                    const string& oldStr,
                    const string& newStr) {
  string::size_type pos = 0u;
  while ((pos = str->find(oldStr, pos)) != string::npos) {
     str->replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}

static const char* automaton_ascii_logo_cstr =
  "\n\x1b[40m\x1b[1m"
  "                                                                     " "\x1b[0m\n\x1b[40m\x1b[1m"
  "                                                                     " "\x1b[0m\n\x1b[40m\x1b[1m"
  "    @197m█▀▀▀█ @39m█ ▀ █ @11m▀▀█▀▀ @129m█▀▀▀█ @47m█▀█▀█ @9m█▀▀▀█ @27m▀▀█▀▀ @154m█▀▀▀█ @13m█▀█ █            " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "    @197m█▀▀▀█ @39m█ ▀ █ @11m▀ █ ▀ @129m█ ▀ █ @47m█ ▀ █ @9m█▀▀▀█ @27m▀ █ ▀ @154m█ ▀ █ @13m█ █ █   @15mCORE     " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "    @197m▀ ▀ ▀ @39m▀▀▀▀▀ @11m▀ ▀ ▀ @129m▀▀▀▀▀ @47m▀ ▀ ▀ @9m▀ ▀ ▀ @27m▀ ▀ ▀ @154m▀▀▀▀▀ @13m▀ ▀▀▀   @15mv0.0.1   " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "                                                                     " "\x1b[0m\n@0m"
  "▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀" "\x1b[0m\n";

int main(int argc, char* argv[]) {
  string automaton_ascii_logo(automaton_ascii_logo_cstr);
  string_replace(&automaton_ascii_logo, "@", "\x1b[38;5;");

{
  engine script;
  script.bind_core();

  // Bind smartproto::node class
  auto node_type = script.create_simple_usertype<node>();

  node_type.set(sol::call_constructor,
    sol::factories(
    [](vector<string> schema_file_names,
       vector<string> script_file_names,
       vector<string> msgs) -> unique_ptr<node> {
      vector<string> schemas_content;
      for (auto schema_file_name : schema_file_names) {
        schemas_content.push_back(get_file_contents(schema_file_name.c_str()));
      }

      vector<string> script_contents;
      for (auto script_file_name : script_file_names) {
        LOG(DEBUG) << "LOAD FILE " << script_file_name;
        script_contents.push_back(get_file_contents(script_file_name.c_str()));
      }

      return make_unique<node>(schemas_content, script_contents, msgs);
    }));

  // Bind this node to its own Lua state.
  node_type.set("add_peer", &node::add_peer);
  node_type.set("remove_peer", &node::remove_peer);
  node_type.set("connect", &node::connect);
  node_type.set("disconnect", &node::disconnect);
  node_type.set("send", &node::send_message);
  node_type.set("listen", &node::set_acceptor);

  node_type.set("msg_id", &node::find_message_id);
  node_type.set("new_msg", &node::create_msg_by_id);
  node_type.set("send", &node::send_message);

  node_type.set("known_peers", [](node& n) {
    LOG(DEBUG) << "getting known peers... " << &n;
    LOG(DEBUG) << n.list_known_peers();
    return sol::as_table(n.list_known_peers());
  });

  node_type.set("peers", [](node& n) {
    LOG(DEBUG) << "getting peers... " << &n;
    LOG(DEBUG) << n.list_connected_peers();
    return sol::as_table(n.list_connected_peers());
  });

  node_type.set("script", &node::script);
  node_type.set("dump_logs", &node::dump_logs);

  script.set_usertype("node", node_type);

  script.script(
      R"(
      function anode()
        return node(
          {"automaton/examples/smartproto/blockchain/blockchain.proto"},
          {
            "automaton/examples/smartproto/blockchain/test.lua",
            "automaton/examples/smartproto/blockchain/blockchain.lua"
          },
          {"Block", "GetBlocks", "Blocks"}
        )
      end

      function BCNode()
        return node(
          {"automaton/examples/smartproto/blockchain/blockchain.proto"},
          {"automaton/examples/smartproto/blockchain/blockchain.lua"},
          {"Block", "GetBlocks", "Blocks"}
        )
      end
      )");

  automaton::core::network::tcp_init();

  automaton::core::network::simulation* sim = automaton::core::network::simulation::get_simulator();
  sim->simulation_start(500);
  automaton::core::cli::cli cli;
  cli.print(automaton_ascii_logo.c_str());
  script.script(get_file_contents("automaton/core/coreinit.lua"));

  cli.history_add("b = BCNode()");
  cli.history_add("dump_logs()");
  cli.history_add("tcp_test()");

  while (1) {
    // auto input = cli.input("\x1b[38;5;15m\x1b[1m 🄰 \x1b[0m ");
    auto input = cli.input("\x1b[38;5;15m\x1b[1m|A|\x1b[0m ");
    if (input == nullptr) {
      cli.print("\n");
      break;
    }

    string cmd{input};
    cli.history_add(cmd.c_str());

    sol::protected_function_result pfr = script.safe_script(cmd, &sol::script_pass_on_error);
    string output = pfr;
    std::cout << output << std::endl;
  }

  // script.safe_script("n1 = nil; n2=nil; collectgarbage()", &sol::script_pass_on_error);
  LOG(DEBUG) << "Destroying lua state & objects";

  sim->simulation_stop();
  delete sim;
}

  LOG(DEBUG) << "tcp_release";

  automaton::core::network::tcp_release();

  LOG(DEBUG) << "tcp_release done.";

  return 0;
}
