#ifndef AUTOMATON_CORE_SMARTPROTO_NODE_H_
#define AUTOMATON_CORE_SMARTPROTO_NODE_H_

#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "automaton/core/data/factory.h"
#include "automaton/core/data/msg.h"
#include "automaton/core/data/schema.h"
#include "automaton/core/io/io.h"
#include "automaton/core/network/acceptor.h"
#include "automaton/core/network/connection.h"
#include "automaton/core/script/engine.h"

namespace automaton {
namespace core {
namespace smartproto {

typedef network::connection_id peer_id;

struct peer_info {
  peer_id id;
  std::string address;
  std::shared_ptr<network::connection> connection;
  std::shared_ptr<char> buffer;
  peer_info();
};

class node: public network::connection::connection_handler,
    public network::acceptor::acceptor_handler {
 public:
  node(std::string id,
       uint32_t update_time_slice,
       std::vector<std::string> schemas,
       std::vector<std::string> lua_scripts,
       std::vector<std::string> wire_msgs,
       std::vector<std::string> commands,
       data::factory& factory); // NOLINT

  /** There must be config.json */
  node(const std::string& id, const std::string& path, data::factory& factory);  // NOLINT

  ~node();

  void init_bindings(std::vector<std::string> schemas,
                     std::vector<std::string> lua_scripts,
                     std::vector<std::string> wire_msgs,
                     std::vector<std::string> commands);

  void init_worker();

  peer_info get_peer_info(peer_id id);

  bool set_peer_info(peer_id id, const peer_info& info);

  void send_message(peer_id id, const data::msg& msg, uint32_t msg_id);

  void send_blob(peer_id id, const std::string& blob, uint32_t msg_id);

  std::string debug_html();

  bool connect(peer_id id);

  bool disconnect(peer_id id);

  bool set_acceptor(const char* address);

  peer_id add_peer(const std::string& address);

  void remove_peer(peer_id id);

  std::vector<peer_id> list_known_peers();

  std::set<peer_id> list_connected_peers();

  // Execute a script which returns corresponding type
  void script(std::string command, std::promise<std::string>* result) {
    add_task([this, command, result]() {
      auto pfr = engine.safe_script(command);
      if (result != nullptr) {
        result->set_value(pfr);
      }
      return "";
    });
  }

  uint32_t find_message_id(const char * name) {
    return engine.get_factory().get_schema_id(name);
  }

  std::unique_ptr<data::msg> create_msg_by_id(uint32_t id) {
    return engine.get_factory().new_message_by_id(id);
  }

  void log(std::string logger, std::string msg);

  void dump_logs(std::string html_file);

  std::string process_cmd(std::string cmd, std::string);

 private:
  std::string nodeid;
  peer_id peer_ids;

  // Script processing related
  script::engine engine;

  sol::protected_function script_on_connected;
  sol::protected_function script_on_disconnected;
  sol::protected_function script_on_update;
  sol::protected_function script_on_msg_sent;
  std::unordered_map<uint32_t, sol::protected_function> script_on_msg;
  std::unordered_map<std::string, sol::protected_function> script_on_cmd;
  sol::protected_function script_on_debug_html;

  uint32_t update_time_slice;

  // Network
  std::shared_ptr<network::acceptor> acceptor_;
  std::mutex peers_mutex;
  std::unordered_map<peer_id, peer_info> known_peers;
  std::set<peer_id> connected_peers;
  std::mutex peer_ids_mutex;

  // Logging
  std::mutex log_mutex;
  std::unordered_map<std::string, std::vector<std::string>> logs;

  peer_id get_next_peer_id();

  bool address_parser(const std::string& s, std::string* protocol, std::string* address);

  // Protocol schema
  std::unordered_map<uint32_t, uint32_t> wire_to_factory;
  std::unordered_map<uint32_t, uint32_t> factory_to_wire;

  // Worker thread
  std::mutex worker_mutex;
  bool worker_stop_signal;
  std::thread* worker;

  std::mutex tasks_mutex;
  std::deque<std::function<std::string()>> tasks;

  void add_task(std::function<std::string()> task) {
    std::lock_guard<std::mutex> lock(tasks_mutex);
    tasks.push_back(task);
  }

  // Inherited handlers' functions

  void on_message_received(peer_id c, char* buffer, uint32_t bytes_read, uint32_t id);

  void on_message_sent(peer_id c, uint32_t id, const common::status& s);

  void on_connected(peer_id c);

  void on_disconnected(peer_id c);

  void on_connection_error(peer_id c, const common::status& s);

  bool on_requested(network::acceptor_id a, const std::string& address, peer_id* id);

  void on_connected(network::acceptor_id a, std::shared_ptr<network::connection> c,
      const std::string& address);

  void on_acceptor_error(network::acceptor_id a, const common::status& s);

  // Script handler functions
  void s_on_blob_received(peer_id id, const std::string& blob);
  void s_on_connected(peer_id id);
  void s_on_disconnected(peer_id id);
  void s_on_error(peer_id id, const std::string& message) {}
};

}  // namespace smartproto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_SMARTPROTO_NODE_H_
