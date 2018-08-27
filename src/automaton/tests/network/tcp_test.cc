
#include <string>
#include <thread>

#include "automaton/core/network/tcp_implementation.h"
#include "automaton/core/io/io.h"

using automaton::core::network::acceptor;
using automaton::core::network::connection;

const char* address_a = "127.0.0.1:12333";
const char* address_b = "127.0.0.1:12366";

std::map<uint32_t, automaton::core::network::connection*> connections;

int counter = 100;
static uint32_t cids = 0;
std::mutex ids_mutex;

uint32_t get_new_id() {
  std::lock_guard<std::mutex> lock(ids_mutex);
  return ++cids;
}

char* bufferA = new char[256];
char* bufferB = new char[256];
char* bufferC = new char[256];

class handler: public connection::connection_handler {
 public:
  void on_message_received(uint32_t c, char* buffer, uint32_t bytes_read, uint32_t mid) {
    std::string message = std::string(buffer, bytes_read);
    LOG(INFO) << "Message \"" << message << "\" received from " << c;
    if (message.compare("Thank you!")) {
      connections[c]->async_send("Thank you!", counter++);
    }
    connections[c]->async_read(buffer, 256, 0);
  }
  void on_message_sent(uint32_t c, uint32_t mid, connection::error e) {
    if (e) {
      LOG(INFO) << "Message with id " << std::to_string(mid) << " was NOT sent to " <<
          c << "\nError " << std::to_string(e) << " occured";
    } else {
      LOG(INFO) << "Message with id " << std::to_string(mid) << " was successfully sent to " << c;
    }
  }
  void on_connected(uint32_t c) {
    LOG(INFO) << "Connected with: " + c;
  }
  void on_disconnected(uint32_t c) {
    LOG(INFO) << "Disconnected with: " + c;
  }
  void on_error(uint32_t c, connection::error e) {
    if (e == connection::no_error) {
      return;
    }
    LOG(ERROR) << std::to_string(e) << " (connection " << c << ")";
  }
};

class lis_handler: public acceptor::acceptor_handler {
 public:
  // TODO(kari): Add constructor that accepts needed options
  // (vector connections, max ...)
  bool on_requested(acceptor* a, const std::string& address, uint32_t* pid) {
  //  EXPECT_EQ(address, address_a);
    *pid = get_new_id();
    LOG(INFO) << "Connection request from: " << address << ". Accepting...";
    return true;
  }
  void on_connected(acceptor* a, connection* c, const std::string& address) {
    LOG(INFO) << "Accepted connection from: " << address;
    connections[c->get_id()] = c;
    char* buffer = new char[256];
    c->async_read(buffer, 256, 0);
  }
  void on_error(acceptor* a, connection::error e) {
    LOG(ERROR) << std::to_string(e);
  }
};

handler handlerC, handlerA;

void func() {
  connection* connection_c = connection::create("tcp", get_new_id(), address_a, &handlerC);
  if (connection_c->init()) {
    connections[connection_c->get_id()] = connection_c;
    LOG(DEBUG) << "Connection init was successful!";
    connection_c -> connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    connection_c -> async_read(bufferC, 256, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    connection_c -> async_send("C0", 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    connection_c -> async_send("C1", 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    connection_c -> async_send("C2", 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    connection_c -> disconnect();
    connection_c -> connect();
    connection_c -> async_read(bufferC, 256, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    connection_c -> async_send("C3", 6);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    connection_c -> async_send("C4", 7);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    connection_c -> async_send("C5", 8);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  } else {
    LOG(ERROR) << "Connection init failed!";
  }
}

int main() {
  automaton::core::network::tcp_init();
  lis_handler lis_handler;
  acceptor* acceptorB = acceptor::create("tcp", address_a, &lis_handler, &handlerA);
  if (acceptorB->init()) {
    LOG(DEBUG) << "Acceptor init was successful!";
    acceptorB->start_accepting();
  } else {
    LOG(ERROR) << "Acceptor init failed!";
  }
  std::thread t(func);

  char r; std::cin >> r;
  t.join();
  automaton::core::network::tcp_release();
  return 0;
}
