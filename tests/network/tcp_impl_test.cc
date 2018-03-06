#include <string>
#include <thread>
#include "network/tcp_implementation.h"

const char* address_a = "127.0.0.1:12333";
const char* address_b = "127.0.0.1:12366";
int counter = 0;
bool passed = true;

class handler: public connection::connection_handler {
 public:
  void on_message_received(connection* c, const std::string& message) {
    logging("Message \"" + message + "\" received from " +
        (reinterpret_cast<tcp_connection*>(c))->get_address());
    if (message.compare("Thank you!")) {
      c->async_send("Thank you!", counter++);
    }
  }
  void on_message_sent(connection* c, int id, connection::error e) {
    if (e) {
      logging("Message with id " + std::to_string(id) + " was NOT sent to " +
          (reinterpret_cast<tcp_connection*>(c))->get_address() + "\nError "
          + std::to_string(e) +" occured");
    } else {
      logging("Message with id " + std::to_string(id) +
          " was successfully sent to "
          + (reinterpret_cast<tcp_connection*>(c))->get_address());
    }
  }
  void on_connected(connection* c) {
    logging("Connected with: " +
        (reinterpret_cast<tcp_connection*>(c))->get_address());
  }
  void on_disconnected(connection* c) {
    logging("Disconnected with: " +
        (reinterpret_cast<tcp_connection*>(c))->get_address());
  }
  void on_error(connection* c, connection::error e) {
    logging("Error: " + std::to_string(e) + " (connection " +
        (reinterpret_cast<tcp_connection*>(c))->get_address() + ")");
  }
};

class lis_handler: public acceptor::acceptor_handler {
 public:
  // TODO(kari): Add constructor that accepts needed options
  // (vector connections, max ...)
  bool on_requested(const std::string& address) {
  //  EXPECT_EQ(address, address_a);
    logging("Connection request from: " + address + ". Accepting...");
    return true;
  }
  void on_connected(connection* c, const std::string& address) {
    logging("Accepted connection from: " + address);
  }
  void on_error(connection::error e) {
    logging("Error (acceptor): " + std::to_string(e));
  }
};
handler _handlerA, _handlerB, _handlerC, _handlerD;
void thread1() {
  connection* _connection_a = connection::create("tcp", address_b, &_handlerA);

  _connection_a -> async_send("A0", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  _connection_a -> async_send("A1", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  _connection_a -> async_send("A2", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  reinterpret_cast<tcp_connection*>(_connection_a) -> disconnect();
}
void thread2() {
  connection* _connection_b = connection::create("tcp", address_b, &_handlerB);
  std::this_thread::sleep_for(std::chrono::milliseconds(60));
  _connection_b -> async_send("B0", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  _connection_b -> async_send("B1", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  _connection_b -> async_send("B2", counter++);
  reinterpret_cast<tcp_connection*>(_connection_b) -> disconnect();
  _connection_b -> async_send("B2", counter++);  // Error
}
void thread3() {
  connection* _connection_c = connection::create("tcp", address_b, &_handlerC);
  _connection_c -> async_send("C0", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  _connection_c -> async_send("C1", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  _connection_c -> async_send("C2", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  reinterpret_cast<tcp_connection*>(_connection_c) -> disconnect();
}

int main() {
  tcp_init();
  lis_handler _lis_handler;
  acceptor* _acceptorB = acceptor::create("tcp", address_b, &_lis_handler,
      &_handlerD);
  std::thread t1(thread1);
  std::thread t2(thread2);
  std::thread t3(thread3);
  if (!passed) {
    return 1;
  }
  int r; std::cin >> r;
  t1.join();
  t2.join();
  t3.join();
  return 0;
}
