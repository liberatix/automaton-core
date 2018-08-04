#include <cstdlib>
#include <mutex>
#include <string>
#include <sstream>
#include "automaton/core/log/log.h"
#include "automaton/core/network/simulated_connection.h"

using automaton::core::network::acceptor;
using automaton::core::network::connection;
using automaton::core::network::simulation;

static int send_counter = 10;
static int read_counter = 0;
std::mutex buffer_mutex;
std::vector<char*> buffers;

char* add_buffer(uint32_t size) {
  std::lock_guard<std::mutex> lock(buffer_mutex);
  buffers.push_back(new char[size]);
  return buffers[buffers.size() - 1];
}
void clear_buffers() {
  std::lock_guard<std::mutex> lock(buffer_mutex);
  for (uint32_t i = 0; i < buffers.size(); ++i) {
    delete [] buffers[i];
  }
}

std::string create_connection_address(uint32_t num_acceptors,
                                      uint32_t min_lag = 1,
                                      uint32_t max_lag = 5,
                                      uint32_t min_bandwidth = 1,
                                      uint32_t max_bandwidth = 4) {
  /// Choosing random min (mn) and max lag (mx): min_lag <= mn < mx <= max_lag
  std::stringstream s;
  uint32_t mn, mx;
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
                                      uint32_t min_connections = 4,
                                      uint32_t max_connections = 16,
                                      uint32_t min_bandwidth = 1,
                                      uint32_t max_bandwidth = 4) {
  std::stringstream s;
  uint32_t conns;
  conns = std::rand() % max_connections;
  conns = conns > min_connections ? conns : min_connections;
  s << conns << ':' << (std::rand() % (max_bandwidth - min_bandwidth + 1) + min_bandwidth) <<
      ':' << address;
  // LOG(INFO) << "Created acceptor address: " << s.str();
  return s.str();
}

class handler: public connection::connection_handler {
 public:
  void on_message_received(connection* c, char* buffer, uint32_t bytes_read, uint32_t id) {
    std::string message = std::string(buffer, bytes_read);
    LOG(INFO) << "Message \"" << message << "\" received in <" << c->get_address() << ">";
    // if (send_counter < 10) {
    //   c -> async_send("Thank you!", send_counter++);
    // }
    c -> async_read(buffer, 16, 0, read_counter++);
  }
  void on_message_sent(connection* c, uint32_t id, connection::error e) {
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
  bool on_requested(acceptor* a, const std::string& address) {
  //  EXPECT_EQ(address, address_a);
    LOG(INFO) << "Connection request from: " << address << ". Accepting...";
    return true;
  }
  void on_connected(acceptor* a, connection* c, const std::string& address) {
    // logging("Accepted connection from: " + address);
    c->async_read(add_buffer(16), 16, 5, read_counter++);
  }
  void on_error(acceptor* a, connection::error e) {
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
    connection_ab -> async_read(add_buffer(16), 16, 4, read_counter++);
    connection* connection_ac = connection::create("sim",
                                              create_connection_address(3),
                                              &handler_);
    connection_ac -> connect();
    connection_ac -> async_read(add_buffer(16), 16, 0, read_counter++);
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
  clear_buffers();
  return 0;
}
