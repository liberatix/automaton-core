#include "automaton/core/smartproto/node.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <regex>
#include <thread>
#include <utility>

#include "automaton/core/io/io.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"

using automaton::core::data::msg;
using automaton::core::data::schema;
using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::data::protobuf::protobuf_schema;

using std::chrono::system_clock;
using std::ios_base;
using std::lock_guard;
using std::make_unique;
using std::mutex;
using std::ofstream;
using std::string;
using std::unique_ptr;
using std::vector;

namespace automaton {
namespace core {
namespace smartproto {

// TODO(kari): Make buffers in connection shared_ptr

static const uint32_t MAX_MESSAGE_SIZE = 512;  // Maximum size of message in bytes
static const uint32_t HEADER_SIZE = 3;
static const uint32_t WAITING_HEADER = 1;
static const uint32_t WAITING_MESSAGE = 2;

peer_info::peer_info(): id(0), address("") {}

static std::string fresult(sol::protected_function_result pfr) {
  if (!pfr.valid()) {
    sol::error err = pfr;
    string what = err.what();
    LOG(ERROR) << "*** SCRIPT ERROR ***\n" << what;
    return what;
  }

  return "";
}

node::node(std::string id,
           vector<string> schemas,
           vector<string> lua_scripts,
           vector<string> wire_msgs)
    : id(id)
    , peer_ids(0)
    , acceptor_(nullptr) {
  LOG(DEBUG) << "Node constructor called";
  engine["node_id"] = (uint64_t)(this);

  for (auto schema_content : schemas) {
    schema* pb_schema = new protobuf_schema(schema_content);
    engine.import_schema(pb_schema);
    engine.bind_core();
  }

  // Bind node methods.
  engine.set_function("send",
    [this](uint32_t peer_id, const core::data::msg& msg, uint32_t msg_id) {
      send_message(peer_id, msg, msg_id);
    });

  engine.set_function("log",
    [this](string logger, string msg) {
      log(logger, msg);
    });

  engine.set_function("nodeid", [this]() {
    return this->id;
  });

  engine.set_function("peer", [this](uint32_t peer_id) {
    
  });

  for (string lua_script : lua_scripts) {
    fresult(engine.safe_script(lua_script, &sol::script_pass_on_error));
  }

  script_on_update = engine["update"];
  script_on_connected = engine["connected"];
  script_on_disconnected = engine["disconnected"];
  script_on_msg_sent = engine["sent"];

  lock_guard<mutex> lock(worker_mutex);
  worker_stop_signal = false;
  worker = new std::thread([this]() {
    while (!worker_stop_signal) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
         std::chrono::system_clock::now().time_since_epoch()).count();

      // Call update function only if a valid definition for it exists.
      if (script_on_update.valid()) {
        fresult(script_on_update(current_time));
      }

      // TODO(asen): custom break condition, e.g. max number of tasks per update.
      // Process tasks pending in the queue.
      while (!tasks.empty()) {
        tasks_mutex.lock();
        auto task = tasks.front();
        tasks.pop_front();
        LOG(DEBUG) << "BEFORE TASK";
        auto result = task();
        LOG(DEBUG) << "AFTER TASK";
        if (result.size() > 0) {
          LOG(DEBUG) << task();
        }
        tasks_mutex.unlock();
      }
    }
  });

  // Map wire msg IDs to factory msg IDs and vice versa.
  uint32_t wire_id = 0;
  for (auto wire_msg : wire_msgs) {
    auto factory_id = engine.get_factory().get_schema_id(wire_msg);
    factory_to_wire[factory_id] = wire_id;
    wire_to_factory[wire_id] = factory_id;
    string function_name = "on_" + wire_msg;
    LOG(DEBUG) << wire_id << ": " << function_name;
    script_on_msg[wire_id] = engine[function_name];
    wire_id++;
  }
}

node::~node() {
  // LOG(DEBUG) << "Node destructor called";
  //
  // vector<peer_id> res = list_known_peers();
  // LOG(DEBUG) << "Known peers " << res.size();
  // for (uint32_t i = 0; i < res.size(); ++i) {
  //   LOG(DEBUG) << "known_peer: " << res[i];
  // }

  lock_guard<mutex> lock(worker_mutex);
  worker_stop_signal = true;
  worker->join();
  delete worker;
}

static string get_date_string(system_clock::time_point t) {
  auto as_time_t = std::chrono::system_clock::to_time_t(t);
  struct tm* tm;
  if (tm = ::gmtime(&as_time_t)) {
    char some_buffer[64];
    if (std::strftime(some_buffer, sizeof(some_buffer), "%F %T", tm)) {
      return string{some_buffer};
    }
  }
  throw std::runtime_error("Failed to get current date as string");
}

string zero_padded(int num, int width) {
  std::ostringstream ss;
  ss << std::setw(width) << std::setfill('0') << num;
  return ss.str();
}

void node::log(string logger, string msg) {
  if (logs.count(logger) == 0) {
    logs.emplace(logger, vector<string>());
  }
  auto now = std::chrono::system_clock::now();
  auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
     now.time_since_epoch()).count();
  logs[logger].push_back(
    "[" + get_date_string(now) + "." + zero_padded(current_time % 1000, 3) + "] " + msg);
}

void node::dump_logs(string html_file) {
  ofstream f;
  f.open(html_file, ios_base::trunc);

  f << R"(
<html>
<head>
<style>
pre {
  border: 1px solid black;
  padding: 8px;
  overflow:auto;
  font-size: 16px;
  font-family: 'Inconsolata', monospace;
}

.button {
  font: bold 11px Play;
  text-decoration: none;
  background-color: #aad8f3;
  color: #333;
  padding: 2px 6px 2px 6px;
  border-top: 1px solid #CCCCCC;
  border-right: 1px solid #333333;
  border-bottom: 1px solid #333333;
  border-left: 1px solid #CCCCCC;
  margin: 4px;
  line-height: 24px;
}
</style>
</head>
<body>
<br/>
)";

  for (auto log : logs) {
    f << "<a class='button' href='#" << log.first << "'>";
    f << log.first << std::endl;
    f << "</a>\n";
  }

  f << "<hr />\n";

  for (auto log : logs) {
    f << "<br/><span class='button' id='" << log.first << "'>" << log.first << "</span>";
    f << "<pre>";
    for (auto msg : log.second) {
      f << msg << "\n";
    }
    f << "</pre>\n";
  }

  f << "</body></html>\n";
  f.close();
}

void node::script(const char* input) {
  LOG(DEBUG) << "Calling script from node " << input;
  string cmd{input};
  sol::protected_function_result pfr = engine.safe_script(cmd, &sol::script_pass_on_error);
  string output = pfr;
  std::cout << output << std::endl;
}

void node::send_message(peer_id id, const core::data::msg& msg, uint32_t msg_id) {
  auto msg_schema_id = msg.get_schema_id();
  CHECK_GT(factory_to_wire.count(msg_schema_id), 0)
      << "Message " << msg.get_message_type()
      << " not part of the protocol";
  auto wire_id = factory_to_wire[msg_schema_id];
  string msg_blob;
  if (msg.serialize_message(&msg_blob)) {
    msg_blob.insert(0, 1, static_cast<char>(wire_id));
    send_blob(id, msg_blob, msg_id);
  } else {
    LOG(DEBUG) << "Could not serialize message!";
  }
}

void node::s_on_blob_received(peer_id id, const string& blob) {
  auto wire_id = blob[0];
  if (script_on_msg.count(wire_id) != 1) {
    LOG(FATAL) << "Invalid wire msg_id sent to us!";
    return;
  }
  CHECK_GT(wire_to_factory.count(wire_id), 0);
  auto msg_id = wire_to_factory[wire_id];
  msg* m = engine.get_factory().new_message_by_id(msg_id).release();
  m->deserialize_message(blob.substr(1));
  add_task([this, wire_id, id, m]() -> string {
    auto r = fresult(script_on_msg[wire_id](id, m));
    delete m;
    return r;
  });
}


void node::send_blob(peer_id id, const string& blob, uint32_t msg_id) {
  LOG(DEBUG) << (acceptor_ ? acceptor_->get_address() : "N/A") << " send message " << core::io::bin2hex(blob) <<
      " to peer " << id;
  uint32_t blob_size = blob.size();
  if (blob_size > MAX_MESSAGE_SIZE) {
    LOG(ERROR) << "Message size is " << blob_size << " and is too big! Max message size is " << MAX_MESSAGE_SIZE;
    return;
  }
  char buffer[3];
  buffer[2] = blob_size & 0xff;
  buffer[1] = (blob_size >> 8) & 0xff;
  buffer[0] = (blob_size >> 16) & 0xff;
  // TODO(kari): Find more effective way to do this
  string new_message = string(buffer, 3) + blob;
  lock_guard<mutex> lock(peers_mutex);
  if (connected_peers.find(id) == connected_peers.end()) {
    LOG(ERROR) << "Peer " << id << " is not connected! Call connect first!";
    return;
  }
  auto it = known_peers.find(id);
  if (it == known_peers.end()) {
    LOG(ERROR) << "Trying to send message to unknown peer " << id;
    return;
  }
  if (it->second.connection) {
    if (it->second.connection->get_state() == core::network::connection::state::connected) {
      it->second.connection->async_send(new_message, msg_id);
    }
  } else {
    LOG(ERROR) << "No connection in peer " << id;
  }
}

void node::s_on_connected(peer_id id) {
  add_task([this, id]() -> string {
    return fresult(script_on_connected(static_cast<uint32_t>(id)));
  });
}

void node::s_on_disconnected(peer_id id) {
  add_task([this, id]() -> string {
    return fresult(script_on_disconnected(static_cast<uint32_t>(id)));
  });
}

bool node::connect(peer_id id) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  lock_guard<mutex> lock(peers_mutex);
  if (connected_peers.find(id) != connected_peers.end()) {
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
    LOG(DEBUG) << "Peer " << id << " is already connected!";
    return false;
  }
  auto it = known_peers.find(id);
  if (it != known_peers.end()) {
    if (!it->second.connection) {
      LOG(ERROR) << "Connection does not exist!";
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
      return false;
    }
    if (it->second.connection->get_state() == core::network::connection::state::disconnected) {
      it->second.connection->connect();
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
      return true;
    }
  } else {
    LOG(ERROR) << "No such peer " << id;
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  return false;
}

bool node::disconnect(peer_id id) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  peers_mutex.lock();
  auto it1 = connected_peers.find(id);
  if (it1 != connected_peers.end()) {
    auto it = known_peers.find(id);
    if (it != known_peers.end()) {
      std::shared_ptr<core::network::connection> connection = (it->second.connection);
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
      peers_mutex.unlock();
      connection->disconnect();
      return true;
    } else {
      // not in known peers
    }
  } else {
    LOG(ERROR) << "Peer " << id << " is not connected!";
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  peers_mutex.unlock();
  return false;
}

bool node::set_acceptor(const char* address) {
  core::network::acceptor* new_acceptor = nullptr;
  try {
    string protocol, addr;
    if (!address_parser(address, &protocol, &addr)) {
      LOG(DEBUG) << "Address was not parsed!";
      return false;
    }
    new_acceptor = core::network::acceptor::create(protocol, addr, this, this);
    if (new_acceptor && !new_acceptor->init()) {
      LOG(DEBUG) << "Acceptor initialization failed! Acceptor was not created!" << address;
      delete new_acceptor;
      return false;
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "Adding acceptor failed! " << address << " Error: " << e.what();
    if (new_acceptor) {
      delete new_acceptor;
    }
    return false;
  }
  if (!new_acceptor) {
    LOG(ERROR) << "Acceptor was not created!";
    return false;
  }
  acceptor_ = std::shared_ptr<core::network::acceptor> (new_acceptor);
  new_acceptor->start_accepting();
  return true;
}

peer_id node::add_peer(const string& address) {
  // TODO(kari): Return 0 on error?
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  lock_guard<mutex> lock(peers_mutex);
  for (auto it = known_peers.begin(); it != known_peers.end(); ++it) {
    if (it->second.address == address) {
      LOG(ERROR) << "Already have peer " << address;
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
      return it->first;
    }
  }
  peer_info info;
  info.address = address;
  info.id = get_next_peer_id();
  info.connection = nullptr;
  info.buffer = std::shared_ptr<char>(new char[MAX_MESSAGE_SIZE], std::default_delete<char[]>());
  core::network::connection* new_connection = nullptr;
  try {
    string protocol, addr;
    if (!address_parser(address, &protocol, &addr)) {
      LOG(DEBUG) << "Address was not parsed! " << address;
    } else {
      new_connection = core::network::connection::create(protocol, info.id, addr, this);
      if (new_connection && !new_connection->init()) {
        LOG(DEBUG) << "Connection initialization failed! Connection was not created!";
        delete new_connection;
      }
    }
  } catch (std::exception& e) {
    LOG(ERROR) << e.what();
    if (new_connection) {
      delete new_connection;
    }
  }
  if (!new_connection) {
    LOG(DEBUG) << "No new connection";
  } else {
    info.connection = std::shared_ptr<core::network::connection> (new_connection);
  }
  known_peers[info.id] = std::move(info);
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  return info.id;
}

void node::remove_peer(peer_id id) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  peers_mutex.lock();
  auto it1 = known_peers.find(id);
  if (it1 != known_peers.end()) {
    auto it2 = connected_peers.find(id);
    if (it2 != connected_peers.end()) {
      std::shared_ptr<core::network::connection> connection = (it1->second.connection);
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
      peers_mutex.unlock();
      connection->disconnect();
      peers_mutex.lock();
    }
    known_peers.erase(it1);
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << id;
  peers_mutex.unlock();
}

vector<peer_id> node::list_known_peers() {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A");
  lock_guard<mutex> lock(peers_mutex);
  vector<peer_id> res;
  for (auto it = known_peers.begin(); it != known_peers.end(); ++it) {
    res.push_back(it->first);
  }
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A");
  return res;
}

std::set<peer_id> node::list_connected_peers() {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A");
  lock_guard<mutex> lock(peers_mutex);
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A");
  return connected_peers;
}

peer_id node::get_next_peer_id() {
  lock_guard<mutex> lock(peer_ids_mutex);
  return ++peer_ids;
}

bool node::address_parser(const string& s, string* protocol, string* address) {
  std::regex rgx_ip("(.+)://(.+)");
  std::smatch match;
  if (std::regex_match(s.begin(), s.end(), match, rgx_ip) &&
      match.size() == 3) {
    *protocol = match[1];
    *address = match[2];
    return true;
  } else {
    LOG(DEBUG) << "match size: " << match.size();
    for (uint32_t i = 0; i < match.size(); i++) {
      LOG(DEBUG) << "match " << i << " -> " << match[i];
    }
    *protocol = "";
    *address = "";
    return false;
  }
}

void node::on_message_received(peer_id c, char* buffer, uint32_t bytes_read, uint32_t id) {
  LOG(DEBUG) << "RECEIVED: " << core::io::bin2hex(string(buffer, bytes_read)) << " from peer " << c;
  switch (id) {
    case WAITING_HEADER: {
      if (bytes_read != HEADER_SIZE) {
        LOG(ERROR) << "Wrong header size received";
        return;
      }
      // TODO(kari): check if this peer still exists, the buffer could be invalid
      // TODO(kari): make this loop
      uint32_t message_size = 0;
      message_size += (buffer[2] & 0x000000ff);
      message_size += ((buffer[1] & 0x000000ff) << 8);
      message_size += ((buffer[0] & 0x000000ff) << 16);
      LOG(DEBUG) << "MESSAGE SIZE: " << message_size;
      if (!message_size || message_size > MAX_MESSAGE_SIZE) {
        LOG(ERROR) << "Invalid message size!";
        return;
      } else {
        lock_guard<mutex> lock(peers_mutex);
        auto it = known_peers.find(c);
        if (it != known_peers.end() && it->second.connection && it->second.connection->get_state() ==
            core::network::connection::state::connected) {
          LOG(DEBUG) << (acceptor_ ? acceptor_->get_address() : "N/A") << " waits message with size " << message_size
              << " from peer " << id;
          it->second.connection->async_read(buffer, MAX_MESSAGE_SIZE, message_size, WAITING_MESSAGE);
        }
      }
    }
    break;
    case WAITING_MESSAGE: {
      string blob = string(buffer, bytes_read);
      lock_guard<mutex> lock(peers_mutex);
      auto it = known_peers.find(c);
      if (it != known_peers.end() && it->second.connection && it->second.connection->get_state() ==
          core::network::connection::state::connected) {
        LOG(DEBUG) << (acceptor_ ? acceptor_->get_address() : "N/A") << " received message " <<
            core::io::bin2hex(blob) << " from peer " << c;
        it->second.connection->async_read(buffer, MAX_MESSAGE_SIZE, HEADER_SIZE, WAITING_HEADER);
        s_on_blob_received(c, blob);
      }
    }
    break;
    default: {}
  }
}

void node::on_message_sent(peer_id c, uint32_t id, network::connection::error e) {
  LOG(DEBUG) << "Message to peer " << c << " with msg_id " << id << " was sent";
  add_task([this, c, id, e]() -> string {
    script_on_msg_sent(c, id, e == network::connection::error::no_error ? true : false);
    return "on_msg_sent";
  });
}

void node::on_connected(peer_id c) {
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c;
  peers_mutex.lock();
  auto it = known_peers.find(c);
  if (it == known_peers.end()) {
    LOG(ERROR) << "Connected to unknown peer " << c << " THIS SHOULD NEVER HAPPEN";
    peers_mutex.unlock();
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c
        << (it->second.address);
    return;
  }
  LOG(DEBUG) << "Connected to " << c;
  connected_peers.insert(c);
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c
      << (it->second.address);
  it->second.connection->async_read(it->second.buffer.get(), MAX_MESSAGE_SIZE, HEADER_SIZE, WAITING_HEADER);
  peers_mutex.unlock();
  s_on_connected(c);
}

void node::on_disconnected(peer_id c) {
  LOG(DEBUG) << c << " -> on_disconnected";
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c;
  peers_mutex.lock();
  auto it = connected_peers.find(c);
  // If the address is not the id
  if (it != connected_peers.end()) {
    connected_peers.erase(it);
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c;
    peers_mutex.unlock();
    s_on_disconnected(c);
  } else {
    LOG(ERROR) << "No such peer " << c;
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " peer " << c;
    peers_mutex.unlock();
  }
}

void node::on_error(peer_id c, network::connection::error e) {
  LOG(DEBUG) << c << " -> on_error " << e;
  remove_peer(c);
}

bool node::on_requested(network::acceptor* a, const string& address, peer_id* id) {
  LOG(DEBUG) << "Requested connection to " << acceptor_->get_address() << " from " << address;
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  lock_guard<mutex> lock(peers_mutex);
  for (auto it = known_peers.begin(); it != known_peers.end(); ++it) {
    if (it->second.address == address) {
      LOG(ERROR) << "Already have peer " << address;
      VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
      return false;
    }
  }
  *id = get_next_peer_id();
  peer_info info;
  info.address = address;
  info.id = *id;
  info.connection = nullptr;
  info.buffer = std::shared_ptr<char>(new char[MAX_MESSAGE_SIZE], std::default_delete<char[]>());
  known_peers[*id] = std::move(info);
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  return true;
}

void node::on_connected(network::acceptor* a, network::connection* c, const string& address) {
  peer_id id = c->get_id();
  LOG(DEBUG) << "Connected in acceptor " << a->get_address() << " peer with id " << id << " (" << address << ')';
  VLOG(9) << "LOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  peers_mutex.lock();
  auto it = known_peers.find(id);
  if (it == known_peers.end()) {
    VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
    peers_mutex.unlock();
    LOG(ERROR) << "Connected to unknown peer " << id << " (" << address << ')' << " THIS SHOULD NEVER HAPPEN";
    return;
  }
  it->second.connection = std::shared_ptr<network::connection> (c);
  LOG(DEBUG) << "Connected to " << address;
  VLOG(9) << "UNLOCK " << this << " " << (acceptor_ ? acceptor_->get_address() : "N/A") << " addr " << address;
  peers_mutex.unlock();
}

void node::on_error(network::acceptor* a, network::connection::error e)  {
  LOG(DEBUG) << a->get_address() << " -> on_error in acceptor";
}

}  // namespace smartproto
}  // namespace core
}  // namespace automaton
