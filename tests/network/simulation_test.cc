#include <cstdlib>
#include <string>
#include <sstream>
#include "network/simulated_connection.h"

static int send_counter = 0;
static int read_counter = 0;

std::string create_connection_address(unsigned int min_lag,
                                      unsigned int max_lag,
                                      unsigned int num_acceptors) {
  /// Choosing random min (mn) and max lag (mx): min_lag <= mn < mx <= max_lag
  std::stringstream s;
  unsigned int mn, mx;
  mn = std::rand()%(max_lag - min_lag) + min_lag;
  mx = std::rand()%(max_lag - min_lag) + min_lag;
  s << (mn < mx ? mn : mx) << ':' << (mn < mx ? mx : mn) << ':';
  /// For test purposes if we have n acceptors, their addresses are in range 1-n
  s << (std::rand()%num_acceptors + 1);
  logging("Created connection address: " + s.str());
  return s.str();
}
std::string create_acceptor_address(unsigned int min_connections,
                                      unsigned int max_connections,
                                      unsigned int min_bandwidth,
                                      unsigned int max_bandwidth,
                                      unsigned int next_address) {
  std::stringstream s;
  unsigned int conns;
  conns = std::rand()%max_connections;
  conns = conns > min_connections ? conns : min_connections;
  s << conns << ':' << (std::rand()%(max_bandwidth - min_bandwidth) + min_bandwidth)
      << ':' << next_address;
  logging("Created acceptor address: " + s.str());
  return s.str();
}

class handler: public connection::connection_handler {
 public:
  void on_message_received(connection* c, char* buffer, unsigned int bytes_read,
      unsigned int id) {
    std::string message = std::string(buffer, bytes_read);
    logging("Message \"" + message + "\" received in <" + c->get_address() + ">");
    if (message.compare("Thank you!")) {
      c->async_send("Thank you!", send_counter++);
    }
    c -> async_read(buffer, 128, 0, read_counter++);
  }
  void on_message_sent(connection* c, unsigned int id, connection::error e) {
    if (e) {
      logging("Message with id " + std::to_string(id) + " was NOT sent to " +
          c->get_address() + "\nError " + std::to_string(e) +" occured");
    } else {
      logging("Message with id " + std::to_string(id) + " was successfully sent to " +
          c->get_address());
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
    logging("Error: " + std::to_string(e) + " (connection " + c->get_address() + ")");
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
    c->async_read(buffer, 128, 0, read_counter++);
  }
  void on_error(connection::error e) {
    logging("Error (acceptor): " + std::to_string(e));
  }
};

int main() {
  handler handler_;
  lis_handler lis_handler_;
  simulation* sim = simulation::get_simulator();
  acceptor* acceptorA = acceptor::create("sim",
                                        create_acceptor_address(1, 64, 4, 8, 1),
                                        &lis_handler_,
                                        &handler_);
  acceptor* acceptorB = acceptor::create("sim",
                                        create_acceptor_address(1, 64, 4, 8, 2),
                                        &lis_handler_,
                                        &handler_);
  acceptorB->start_accepting();
  acceptorA->start_accepting();
  acceptor* acceptorC = acceptor::create("sim",
                                        create_acceptor_address(1, 64, 4, 8, 3),
                                        &lis_handler_,
                                        &handler_);
  acceptorC->start_accepting();
  connection* connection_ab = connection::create("sim",
                                            create_connection_address(1, 15, 3),
                                            &handler_);
  connection_ab -> connect();
  connection_ab -> async_read(new char[128], 128, 0, read_counter++);
  connection* connection_ac = connection::create("sim",
                                            create_connection_address(1, 15, 3),
                                            &handler_);
  connection_ac -> connect();
  connection_ac -> async_read(new char[128], 128, 0, read_counter++);
  connection_ac -> async_send("B0", send_counter++);
  // connection_bc -> async_send("B1", send_counter++);
  // connection_bc -> async_send("B2", send_counter++);
  // connection_ac -> disconnect();
  // connection_bc -> async_send("B2", send_counter++);  // Error
  connection_ab -> async_send("A0", send_counter++);
  for (int i = 0; i <= 35; i++) {
    logging("PROCESSING: " + std::to_string(i));
    sim->process(i);
  }
  /*
  connection_ab -> async_send("A1", send_counter++);
  connection_ab -> async_send("A2", send_counter++);

  /*connection* connection_bc = connection::create("sim", address_c, &handlerB);
  connection_bc -> connect();
  connection_bc -> async_read(buffer_bc, 128, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  connection_bc -> async_send("C0", send_counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  connection_bc -> async_send("C1", send_counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  connection_bc -> async_send("C2", send_counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  connection_bc -> disconnect();
  connection_bc -> connect();
  connection_bc -> async_read(buffer_bc, 128, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  connection_bc -> async_send("C0", send_counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  connection_bc -> async_send("C1", send_counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  connection_bc -> async_send("C2", send_counter++);
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  connection_ab->disconnect();*/
  // sim->print_q();
  return 0;
}
