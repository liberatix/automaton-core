
#include <string>
#include <thread>

#include "automaton/core/network/tcp_implementation.h"
#include "automaton/core/log/log.h"

using automaton::core::network::acceptor;
using automaton::core::network::connection;

const char* address_a = "127.0.0.1:12333";
const char* address_b = "127.0.0.1:12366";
int counter = 100;
// bool passed = true;
char* bufferA = new char[256];
char* bufferB = new char[256];
char* bufferC = new char[256];

class handler: public connection::connection_handler {
 public:
  void on_message_received(connection* c, char* buffer, uint32_t bytes_read, uint32_t id) {
    std::string message = std::string(buffer, bytes_read);
    LOG(INFO) << "Message \"" << message << "\" received from " << c->get_address();
    if (message.compare("Thank you!")) {
      c->async_send("Thank you!", counter++);
    }
    c -> async_read(buffer, 256, 0);
  }
  void on_message_sent(connection* c, uint32_t id, connection::error e) {
    if (e) {
      LOG(INFO) << "Message with id " << std::to_string(id) << " was NOT sent to " <<
          c->get_address() << "\nError " << std::to_string(e) << " occured";
    } else {
      LOG(INFO) << "Message with id " << std::to_string(id) <<
          " was successfully sent to " << c->get_address();
    }
  }
  void on_connected(connection* c) {
    LOG(INFO) << "Connected with: " + c->get_address();
  }
  void on_disconnected(connection* c) {
    LOG(INFO) << "Disconnected with: " + c->get_address();
  }
  void on_error(connection* c, connection::error e) {
    if (e == connection::no_error) {
      return;
    }
    LOG(ERROR) << std::to_string(e) << " (connection " << c->get_address() << ")";
  }
};

class lis_handler: public acceptor::acceptor_handler {
 public:
  // TODO(kari): Add constructor that accepts needed options
  // (vector connections, max ...)
  bool on_requested(acceptor* a, const std::string& address) {
  //  EXPECT_EQ(address, address_a);
    LOG(INFO) << "Connection request from: " << address << ". Accepting...";
    return true;
  }
  void on_connected(acceptor* a, connection* c, const std::string& address) {
    LOG(INFO) << "Accepted connection from: " << address;
    char* buffer = new char[256];
    c->async_read(buffer, 256, 0);
  }
  void on_error(acceptor* a, connection::error e) {
    LOG(ERROR) << std::to_string(e);
  }
};

handler handlerA, handlerB, handlerC, handlerD;
// void thread1() {
//   connection* connection_a = connection::create("tcp", address_b, &handlerA);
//   connection_a->init();
//   connection_a -> connect();
//   connection_a -> async_read(bufferA, 256, 0);
//   connection_a -> async_send("A0", 1);
//   std::this_thread::sleep_for(std::chrono::milliseconds(300));
//   /*
//   connection_a -> async_send("A1", counter++);
//   std::this_thread::sleep_for(std::chrono::milliseconds(200));
//   connection_a -> async_send("A2", counter++);
//   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//   */
//   connection_a->disconnect();
// }
// void thread2() {
//   connection* connection_b = connection::create("tcp", address_b, &handlerB);
//   connection_b->init();
//   connection_b -> connect();
//   connection_b -> async_read(bufferB, 256, 0);
//   std::this_thread::sleep_for(std::chrono::milliseconds(60));
//   connection_b -> async_send("B0", 2);
//   std::this_thread::sleep_for(std::chrono::milliseconds(100));
//   // connection_b -> async_send("B1", counter++);
//   std::this_thread::sleep_for(std::chrono::milliseconds(50));
//   // connection_b -> async_send("B2", counter++);
//   connection_b -> disconnect();
//   // connection_b -> async_send("B2", counter++);  // Error
// }
void thread3() {
  connection* connection_c = connection::create("tcp", address_b, &handlerC);
  if (connection_c->init()) {
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
  acceptor* acceptorB = acceptor::create("tcp", address_b, &lis_handler,
      &handlerD);
  if (acceptorB->init()) {
    LOG(DEBUG) << "Acceptor init was successful!";
    acceptorB->start_accepting();
  } else {
    LOG(ERROR) << "Acceptor init failed!";
  }
  //  std::thread t1(thread1);
  //  std::thread t2(thread2);
  std::thread t3(thread3);
  /*if (!passed) {
    return 1;
  }*/
  char r; std::cin >> r;
  //  t1.join();
  //  t2.join();
  t3.join();
  return 0;
}
