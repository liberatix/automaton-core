#include <string>
#include "network/simulated_connection.h"

const char* address_a = "0:8:8:1:1:2";
const char* address_b = "1:2:8:8:1:3";
const char* address_c = "0:5:8:1:2:3";
int counter = 0;

char* buffer_ab = new char[128];
char* buffer_ac = new char[128];
char* buffer_bc = new char[128];

class handler: public connection::connection_handler {
 public:
  void on_message_received(connection* c, char* buffer, unsigned int bytes_read,
      unsigned int id) {
    std::string message = std::string(buffer, bytes_read);
    logging("Message \"" + message + "\" received in <" + c->get_address()
        + ">");
    if (message.compare("Thank you!")) {
      c->async_send("Thank you!", counter++);
    }
    c -> async_read(buffer, 128, 0, counter++);
  }
  void on_message_sent(connection* c, unsigned int id, connection::error e) {
    if (e) {
      logging("Message with id " + std::to_string(id) + " was NOT sent to " +
          c->get_address() + "\nError " + std::to_string(e) +" occured");
    } else {
      logging("Message with id " + std::to_string(id) +
          " was successfully sent to " + c->get_address());
    }
  }
  void on_connected(connection* c) {
  //  logging("Connected with: " + c->get_address());
  }
  void on_disconnected(connection* c) {
  //  logging("Disconnected with: " + c->get_address());
  }
  void on_error(connection* c, connection::error e) {
    if (e == connection::no_error) {
      return;
    }
    logging("Error: " + std::to_string(e) + " (connection " + c->get_address() +
        ")");
  }
};

class lis_handler: public acceptor::acceptor_handler {
 public:
  // TODO(kari): Add constructor that accepts needed options
  // (vector connections, max ...)
  bool on_requested(const std::string& address) {
  //  EXPECT_EQ(address, address_a);
  // logging("Connection request from: " + address + ". Accepting...");
    return true;
  }
  void on_connected(connection* c, const std::string& address) {
    // logging("Accepted connection from: " + address);
    char* buffer = new char[128];
    c->async_read(buffer, 128, 0, counter++);
  }
  void on_error(connection::error e) {
    logging("Error (acceptor): " + std::to_string(e));
  }
};

int main() {
  handler handlerA, handlerB, handlerC;
  lis_handler lis_handlerA, lis_handlerB, lis_handlerC;
  simulation* sim = simulation::get_simulator();
  acceptor* acceptorA = acceptor::create("sim", "1", &lis_handlerA,
      &handlerA);
  acceptor* acceptorB = acceptor::create("sim", "2", &lis_handlerB,
      &handlerB);
  acceptorB->start_accepting();
  acceptorA->start_accepting();
  acceptor* acceptorC = acceptor::create("sim", "3", &lis_handlerC,
    &handlerC);
  acceptorC->start_accepting();
  connection* connection_ab = connection::create("sim", address_a, &handlerA);
  connection_ab -> connect();
  connection_ab -> async_read(buffer_ab, 128, 0, counter++);
  connection* connection_ac = connection::create("sim", address_b, &handlerA);
  connection_ac -> connect();
  connection_ac -> async_read(buffer_ac, 128, 0, counter++);
  connection_ac -> async_send("B0", counter++);
  for (int i = 0; i <= 5; i++) {
    logging("PROCESSING: " + std::to_string(i));
    sim->process(i);
  }
  // connection_bc -> async_send("B1", counter++);
  // connection_bc -> async_send("B2", counter++);
  // connection_ac -> disconnect();
  // connection_bc -> async_send("B2", counter++);  // Error
  connection_ab -> async_send("A0", counter++);
  for (int i = 5; i <= 20; i++) {
    logging("PROCESSING: " + std::to_string(i));
    sim->process(i);
  }
  /*
  connection_ab -> async_send("A1", counter++);
  connection_ab -> async_send("A2", counter++);

  /*connection* connection_bc = connection::create("sim", address_c, &handlerB);
  connection_bc -> connect();
  connection_bc -> async_read(buffer_bc, 128, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  connection_bc -> async_send("C0", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  connection_bc -> async_send("C1", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  connection_bc -> async_send("C2", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  connection_bc -> disconnect();
  connection_bc -> connect();
  connection_bc -> async_read(buffer_bc, 128, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  connection_bc -> async_send("C0", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  connection_bc -> async_send("C1", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  connection_bc -> async_send("C2", counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  connection_ab->disconnect();*/
  // sim->print_q();
  return 0;
}
