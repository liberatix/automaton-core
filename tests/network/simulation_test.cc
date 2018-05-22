#include <cstdlib>
#include <string>
#include <sstream>
#include "log/log.h"
#include "network/simulated_connection.h"

static int send_counter = 10;
static int read_counter = 0;

std::string create_connection_address(unsigned int num_acceptors,
                                      unsigned int min_lag = 1,
                                      unsigned int max_lag = 5,
                                      unsigned int min_bandwidth = 1,
                                      unsigned int max_bandwidth = 4) {
  /// Choosing random min (mn) and max lag (mx): min_lag <= mn < mx <= max_lag
  std::stringstream s;
  unsigned int mn, mx;
  if (min_lag == max_lag) {
    s << min_lag << ':' << max_lag << ':';
  } else {
    mn = std::rand() % (max_lag - min_lag + 1) + min_lag;
    mx = std::rand() % (max_lag - min_lag + 1) + min_lag;
    s << (mn < mx ? mn : mx) << ':' << (mn < mx ? mx : mn) << ':';
  }
  /// For test purposes if we have n acceptors, their addresses are in range 1-n
  s << (std::rand() % (max_bandwidth - min_bandwidth + 1) + min_bandwidth) << ':' <<
      (std::rand() % num_acceptors + 1);
  // LOG(INFO) << "Created connection address: " << s.str();
  return s.str();
}
std::string create_acceptor_address(uint32_t address,
                                      unsigned int min_connections = 4,
                                      unsigned int max_connections = 16,
                                      unsigned int min_bandwidth = 1,
                                      unsigned int max_bandwidth = 4) {
  std::stringstream s;
  unsigned int conns;
  conns = std::rand() % max_connections;
  conns = conns > min_connections ? conns : min_connections;
  s << conns << ':' << (std::rand() % (max_bandwidth - min_bandwidth + 1) + min_bandwidth) <<
      ':' << address;
  // LOG(INFO) << "Created acceptor address: " << s.str();
  return s.str();
}

class handler: public connection::connection_handler {
 public:
  void on_message_received(connection* c, char* buffer, unsigned int bytes_read, unsigned int id) {
    std::string message = std::string(buffer, bytes_read);
    LOG(INFO) << "Message \"" << message << "\" received in <" << c->get_address() << ">";
    // if (send_counter < 10) {
    //   c -> async_send("Thank you!", send_counter++);
    // }
    c -> async_read(buffer, 128, 0, read_counter++);
  }
  void on_message_sent(connection* c, unsigned int id, connection::error e) {
    if (e) {
      LOG(ERROR) << "Message with id " << std::to_string(id) << " was NOT sent to " <<
          c->get_address() << "\nError " << std::to_string(e) << " occured";
    } else {
      LOG(INFO) << "Message with id " << std::to_string(id) << " was successfully sent to " <<
          c->get_address();
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
    LOG(ERROR) << std::to_string(e) << " (connection " + c->get_address() << ")";
  }
};

class lis_handler: public acceptor::acceptor_handler {
 public:
  // TODO(kari): Add constructor that accepts needed options
  // (vector connections, max ...)
  bool on_requested(const std::string& address) {
  //  EXPECT_EQ(address, address_a);
    LOG(INFO) << "Connection request from: " << address << ". Accepting...";
    return true;
  }
  void on_connected(connection* c, const std::string& address) {
    // logging("Accepted connection from: " + address);
    c->async_read(new char[128], 128, 5, read_counter++);
  }
  void on_error(connection::error e) {
    LOG(ERROR) << std::to_string(e);
  }
};

int main() {
  try {
    handler handler_;
    lis_handler lis_handler_;
    simulation* sim = simulation::get_simulator();
    acceptor* acceptorA = acceptor::create("sim",
                                          create_acceptor_address(1),
                                          &lis_handler_,
                                          &handler_);
    acceptor* acceptorB = acceptor::create("sim",
                                          create_acceptor_address(2),
                                          &lis_handler_,
                                          &handler_);
    acceptorB->start_accepting();
    acceptorA->start_accepting();
    acceptor* acceptorC = acceptor::create("sim",
                                          create_acceptor_address(3),
                                          &lis_handler_,
                                          &handler_);
    acceptorC->start_accepting();
    connection* connection_ab = connection::create("sim",
                                              create_connection_address(3),
                                              &handler_);
    connection_ab -> connect();
    connection_ab -> async_read(new char[128], 128, 4, read_counter++);
    connection* connection_ac = connection::create("sim",
                                              create_connection_address(3),
                                              &handler_);
    connection_ac -> connect();
    connection_ac -> async_read(new char[128], 128, 0, read_counter++);
    connection_ac -> async_send("B0", 0);
    connection_ac -> async_send("B1", 1);
    connection_ab -> async_send("A0", 2);
    // LOG(INFO) << "FIRST PROCESS LOOP";
    for (int i = sim->get_time(); !sim->is_queue_empty(); i++) {
      // LOG(INFO) << "PROCESSING " << i;
      sim->process(i);
    }
    connection_ab -> disconnect();
    connection_ac -> async_send("B2", 3);
    // LOG(INFO) << "SECOND PROCESSING LOOP";
    for (int i = sim->get_time(); !sim->is_queue_empty(); i++) {
      // LOG(INFO) << "PROCESSING " << i;
      sim->process(i);
    }
    connection_ab -> connect();
    connection_ab -> async_send("A1", 4);
    connection_ab -> async_send("A2", 5);
    // LOG(INFO) << "THIRD PROCESSING LOOP";
    for (int i = sim->get_time(); !sim->is_queue_empty(); i++) {
      // LOG(INFO) << "PROCESSING " << i;
      sim->process(i);
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "EXCEPTION " + std::string(e.what());
  } catch(...) {
    LOG(ERROR) << "UNKNOWN EXCEPTION!";
  }
  return 0;
}
