#include <future>
#include <iostream>
#include <regex>
#include <string>

#include <json.hpp>

#include "automaton/core/network/tcp_implementation.h"
#include "automaton/core/cli/cli.h"
#include "automaton/core/data/factory.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "automaton/core/io/io.h"
#include "automaton/core/network/simulated_connection.h"
#include "automaton/core/script/engine.h"
#include "automaton/core/node/node.h"
#include "automaton/core/smartproto/smart_protocol.h"

using automaton::core::data::factory;
using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::data::schema;
using automaton::core::io::get_file_contents;
using automaton::core::script::engine;
using automaton::core::smartproto::node;
using automaton::core::smartproto::smart_protocol;

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
  "    @197m█▀▀▀█ @39m█ █ █ @11m▀▀█▀▀ @129m█▀▀▀█ @47m█▀█▀█ @9m█▀▀▀█ @27m▀▀█▀▀ @154m█▀▀▀█ @13m█▀█ █            " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "    @197m█▀▀▀█ @39m█ ▀ █ @11m█ █ █ @129m█ ▀ █ @47m█ ▀ █ @9m█▀▀▀█ @27m█ █ █ @154m█ ▀ █ @13m█ █ █   @15mCORE     " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "    @197m▀ ▀ ▀ @39m▀▀▀▀▀ @11m▀ ▀ ▀ @129m▀▀▀▀▀ @47m▀ ▀ ▀ @9m▀ ▀ ▀ @27m▀ ▀ ▀ @154m▀▀▀▀▀ @13m▀ ▀▀▀   @15mv0.0.1   " "\x1b[0m\n\x1b[40m\x1b[1m" // NOLINT
  "                                                                     " "\x1b[0m\n@0m"
  "▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀" "\x1b[0m\n";

int main(int argc, char* argv[]) {
  string automaton_ascii_logo(automaton_ascii_logo_cstr);
  string_replace(&automaton_ascii_logo, "@", "\x1b[38;5;");

  vector<unique_ptr<factory>> factories;

{
  automaton::core::cli::cli cli;

  auto core_factory = make_unique<protobuf_factory>();
  engine script(*core_factory);
  factories.push_back(std::move(core_factory));
  script.bind_core();

  // Bind smartproto::node class
  auto node_type = script.create_simple_usertype<node>();

  node_type.set(sol::call_constructor,
    sol::factories(
    [&factories](string id,
       uint32_t update_time_slice,
       vector<schema*> schemas,
       vector<string> scripts,
       vector<string> msgs,
       vector<string> commands) -> unique_ptr<node> {
      auto core_factory = make_unique<protobuf_factory>();
      auto core_ptr = core_factory.get();
      factories.push_back(std::move(core_factory));
      return make_unique<node>(
          id, update_time_slice, schemas, scripts, msgs, commands, *core_ptr);
    },
    [&factories](const std::string& id, std::string proto) -> unique_ptr<node> {
      auto core_factory = make_unique<protobuf_factory>();
      auto core_ptr = core_factory.get();
      factories.push_back(std::move(core_factory));
      return make_unique<node>(id, proto, *core_ptr);
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

  node_type.set("dump_logs", &node::dump_logs);
  node_type.set("debug_html", &node::debug_html);

  node_type.set("script", [](node& n, std::string command) -> std::string {
    std::promise<std::string> prom;
    std::future<std::string> fut = prom.get_future();
    n.script(command, &prom);
    std::string result = fut.get();
    return result;
  });

  node_type.set("process_cmd", &node::process_cmd);

  node_type.set("call", [](node& n, std::string command) {
    n.script(command, nullptr);
  });

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

  script.set_usertype("node", node_type);

  script.set_function("history_add", [&](std::string cmd){
    cli.history_add(cmd.c_str());
  });

  automaton::core::network::tcp_init();

  std::shared_ptr<automaton::core::network::simulation> sim = automaton::core::network::simulation::get_simulator();
  sim->simulation_start(100);
  cli.print(automaton_ascii_logo.c_str());

  script.safe_script(get_file_contents("automaton/examples/smartproto/common/names.lua"));
  script.safe_script(get_file_contents("automaton/examples/smartproto/common/dump.lua"));
  script.safe_script(get_file_contents("automaton/examples/smartproto/common/network.lua"));
  script.safe_script(get_file_contents("automaton/examples/smartproto/common/connections_graph.lua"));
  script.safe_script(get_file_contents("automaton/examples/smartproto/common/show_states.lua"));

  std::unordered_map<std::string, std::pair<std::string, std::string> > rpc_commands;

  std::ifstream i("automaton/core/coreinit.json");
  if (!i.is_open()) {
    LOG(ERROR) << "coreinit.json could not be opened";
  } else {
    nlohmann::json j;
    i >> j;
    i.close();
    std::vector<std::string> paths = j["protocols"];
    for (auto p : paths) {
      script.safe_script(get_file_contents((p + "init.lua").c_str()));
      smart_protocol* protocol = new smart_protocol();
      protocol->load(p);
    }
    script.set_function("get_core_supported_protocols", [&](){
      std::unordered_map<std::string, std::unordered_map<std::string, std::string> > protocols;
      for (std::string proto : smart_protocol::list_protocols()) {
        protocols[proto] = smart_protocol::get_protocol(proto)->get_msgs_definitions();
      }
      return sol::as_table(protocols);
    });

    std::vector<std::string> rpc_protos = j["command_definitions"];
    for (auto p : rpc_protos) {
      schema* rpc_schema = new protobuf_schema(get_file_contents(p.c_str()));
      script.import_schema(rpc_schema);
    }
    std::vector<std::string> rpc_luas = j["command_implementations"];
    for (auto p : rpc_luas) {
      script.safe_script(get_file_contents(p.c_str()));
    }
    for (auto c : j["commands"]) {
      std::cout << "loaded rpc command: " << c["cmd"] << std::endl;
      rpc_commands[c["cmd"]] = std::make_pair(c["input"], c["output"]);
    }
  }
  i.close();

  // Start dump_logs thread.
  std::mutex logger_mutex;
  bool stop_logger = false;
  std::thread logger([&]() {
    while (!stop_logger) {
      // Dump logs once per second.
      std::this_thread::sleep_for(std::chrono::milliseconds(1500));
      logger_mutex.lock();
      try {
        sol::protected_function_result pfr;
        pfr = script.safe_script(
          R"(for k,v in pairs(networks) do
              dump_logs(k)
            end
          )");
        if (!pfr.valid()) {
          sol::error err = pfr;
          std::cout << "\n" << err.what() << "\n";
          break;
        }
      } catch (std::exception& e) {
        LOG(FATAL) << "Exception in logger: " << e.what();
      } catch (...) {
        LOG(FATAL) << "Exception in logger";
      }
      logger_mutex.unlock();
    }
  });

  while (1) {
    // auto input = cli.input("\x1b[38;5;15m\x1b[1m 🄰 \x1b[0m ");
    auto input = cli.input("\x1b[38;5;15m\x1b[1m|A|\x1b[0m ");
    if (input == nullptr) {
      cli.print("\n");
      break;
    }

    string cmd{input};
    cli.history_add(cmd.c_str());

    logger_mutex.lock();
    sol::protected_function_result pfr = script.safe_script(cmd);
    logger_mutex.unlock();

    if (!pfr.valid()) {
      sol::error err = pfr;
      std::cout << "\n" << err.what() << "\n";
    }
  }

  stop_logger = true;
  logger.join();

  LOG(DEBUG) << "Destroying lua state & objects";

  sim->simulation_stop();
}

  LOG(DEBUG) << "tcp_release";

  automaton::core::network::tcp_release();

  LOG(DEBUG) << "tcp_release done.";

  return 0;
}
