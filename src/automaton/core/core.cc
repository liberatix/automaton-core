#include <iostream>
#include <regex>
#include <string>

#include <json.hpp>

#include "automaton/core/network/tcp_implementation.h"
#include "automaton/core/cli/cli.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "automaton/core/io/io.h"
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
  "                                                                     " "\x1b[0m\n\x1b[40m\x1b[1m"
  "                                                                     " "\x1b[0m\n\x1b[40m\x1b[1m"
  "    @197m█▀▀▀█ @39m█ ▀ █ @11m▀▀█▀▀ @129m█▀▀▀█ @47m█▀█▀█ @9m█▀▀▀█ @27m▀▀█▀▀ @154m█▀▀▀█ @13m█▀█ █            " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "    @197m█▀▀▀█ @39m█ ▀ █ @11m▀ █ ▀ @129m█ ▀ █ @47m█ ▀ █ @9m█▀▀▀█ @27m▀ █ ▀ @154m█ ▀ █ @13m█ █ █   @15mCORE     " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "    @197m▀ ▀ ▀ @39m▀▀▀▀▀ @11m▀ ▀ ▀ @129m▀▀▀▀▀ @47m▀ ▀ ▀ @9m▀ ▀ ▀ @27m▀ ▀ ▀ @154m▀▀▀▀▀ @13m▀ ▀▀▀   @15mv0.0.1   " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "                                                                     " "\x1b[0m\n@0m"
  "▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀" "\x1b[0m\n";

int main(int argc, char* argv[]) {
  std::string automaton_ascii_logo(automaton_ascii_logo_cstr);
  string_replace(&automaton_ascii_logo, "@", "\x1b[38;5;");

  {
    sol::state lua;
    lua.open_libraries();

    struct ttt {
      int x;
      int y;
      ttt() {
        x = 1;
        y = 2;
      }
      ~ttt() {
        std::cout << "Destroying ttt\n";
      }
    };

    auto ttt_type = lua.create_simple_usertype<ttt>();
    ttt_type.set(sol::call_constructor,
      sol::factories([]() -> unique_ptr<ttt> {
        return make_unique<ttt>();
      }));

    ttt_type.set("x", &ttt::x);
    ttt_type.set("y", &ttt::y);
    lua.set_usertype("ttt", ttt_type);
    lua.script("a = ttt.new(); print(a.x, a.y); a.x = 5; a.y = 5; print(a.x, a.y)");
    // lua.script("a = nil; collectgarbage()");
    std::cout << "Destroying lua engine\n";
  }

{
  lua_script_engine engine;
  engine.bind_core();
  sol::state_view& lua = engine.get_sol();

  // Bind smartproto::node class
  auto node_type = lua.create_simple_usertype<node>();

  node_type.set(sol::call_constructor,
    sol::factories(
    [](const char* schema_file_name, const char* proto_file_name)
    -> unique_ptr<node> {
      auto schema_contents = get_file_contents(schema_file_name);
      auto script_contents = get_file_contents(proto_file_name);
      unique_ptr<schema> pb_schema(new protobuf_schema(schema_contents));
      return make_unique<node>(std::move(pb_schema), script_contents);
    }));

  // Bind this node to its own Lua state.
  node_type.set("add_peer", &node::add_peer);
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

  lua.set_usertype("node", node_type);

  lua.script(
      R"(
      function anode()
        return node(
          "automaton/examples/smartproto/blockchain/blockchain.proto",
          "automaton/examples/smartproto/blockchain/test.lua"
        )
      end
      )");

  automaton::core::network::tcp_init();

  automaton::core::cli::cli cli;
  lua.script(get_file_contents("automaton/core/coreinit.lua"));

  cli.print(automaton_ascii_logo.c_str());

  while (1) {
    // auto input = cli.input("\x1b[38;5;15m\x1b[1m 🄰 \x1b[0m ");
    auto input = cli.input("\x1b[38;5;15m\x1b[1m|A|\x1b[0m ");
    if (input == nullptr) {
      cli.print("\n");
      break;
    }

    std::string cmd{input};
    cli.history_add(cmd.c_str());

    sol::protected_function_result pfr = lua.safe_script(cmd, &sol::script_pass_on_error);
    std::string output = pfr;
    std::cout << output << std::endl;
  }

  // lua.safe_script("n1 = nil; n2=nil; collectgarbage()", &sol::script_pass_on_error);
  LOG(DEBUG) << "Destroying lua state & objects";
}

  LOG(DEBUG) << "tcp_release";

  automaton::core::network::tcp_release();

  LOG(DEBUG) << "tcp_release done.";

  return 0;
}
