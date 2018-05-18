#include <cstdlib>
#include <string>
#include <sstream>

#include "log/log.h"
#include "network/simulated_connection.h"

/// Constants

static const unsigned int NUMBER_NODES = 100000;
// These include only the peers that a node connects to, not the accepted ones
static const unsigned int NUMBER_PEERS_IN_NODE = 4;
static const unsigned int MIN_LAG = 100;
static const unsigned int MAX_LAG = 1000;
static const unsigned int MIN_CONNECTIONS = 0;
static const unsigned int MAX_CONNECTIONS = 1;
static const unsigned int MIN_BANDWIDTH = 16;
static const unsigned int MAX_BANDWIDTH = 16;
static const unsigned int LOOP_STEP = 100;
static const unsigned int PROCESS_STEP = 100;
static const unsigned int BLOCK_CREATION_STEP = 1500;
static const unsigned int MAX_SIMULATION_TIME = 10000;

/// Global variables

class node;
/// height -> how many connections have that height
static std::map<unsigned int, unsigned int> heights_count;
static std::vector<node*> nodes(NUMBER_NODES);

/// Helper functions for creating addresses

std::string create_connection_address(unsigned int num_acceptors, unsigned int this_acceptor,
                                      unsigned int min_lag, unsigned int max_lag) {
  /// Choosing random min (mn) and max lag (mx): min_lag <= mn < mx <= max_lag
  std::stringstream s;
  unsigned int mn, mx, acc;
  if (min_lag == max_lag) {
    s << min_lag << ':' << max_lag << ':';
  } else {
    mn = std::rand() % (max_lag - min_lag + 1) + min_lag;
    mx = std::rand() % (max_lag - min_lag + 1) + min_lag;
    s << (mn < mx ? mn : mx) << ':' << (mn < mx ? mx : mn) << ':';
  }
  /// For test purposes if we have n acceptors, their addresses are in range 1-n
  do {
    acc = (std::rand() % num_acceptors + 1);
  } while (acc == this_acceptor);
  s << acc;
  // logging("Created connection address: " + s.str());
  return s.str();
}
std::string create_connection_address(unsigned int num_acceptors, unsigned int this_acceptor) {
  return create_connection_address(num_acceptors, this_acceptor, MIN_LAG, MAX_LAG);
}
std::string create_acceptor_address(uint32_t address,
                                    unsigned int min_connections, unsigned int max_connections,
                                    unsigned int min_bandwidth, unsigned int max_bandwidth) {
  std::stringstream s;
  unsigned int conns;
  conns = std::rand() % max_connections;
  conns = conns > min_connections ? conns : min_connections;
  s << conns << ':' << (std::rand() % (max_bandwidth - min_bandwidth + 1) + min_bandwidth) << ':'
      << address;
  // logging("Created acceptor address: " + s.str());
  return s.str();
}
std::string create_acceptor_address(uint32_t address) {
  return create_acceptor_address(address, MIN_CONNECTIONS, MAX_CONNECTIONS, MIN_BANDWIDTH,
                                MAX_BANDWIDTH);
}

/// Class node
class node {
 public:
  unsigned int height;
  acceptor* acceptor_;
  connection::connection_handler* handler_;
  std::vector<connection*> peers;
  void send_height() {
    for (unsigned int i = 0; i < peers.size(); ++i) {
      peers[i]->async_send(std::to_string(height), 0);
    }
  }
};

/// Connection handler
class handler: public connection::connection_handler {
 public:
  node* node_;
  explicit handler(node* n): node_(n) {}
  void on_message_received(connection* c, char* buffer, unsigned int bytes_read, unsigned int id) {
    std::string message = std::string(buffer, bytes_read);
    // logging("Message \"" + message + "\" received in <" + c->get_address() + ">");
    if (std::stoul(message) > node_->height) {
      node_->height = std::stoul(message);
      node_->send_height();
    }
    c -> async_read(buffer, 16, 0, 0);
  }
  void on_message_sent(connection* c, unsigned int id, connection::error e) {
    if (e) {
       LOG(ERROR) << "Message with id " << std::to_string(id) << " was NOT sent to " <<
          c->get_address() << "\nError " << std::to_string(e);
    } else {
      // logging("Message with id " + std::to_string(id) + " was successfully sent to " +
      //    c->get_address());
    }
  }
  void on_connected(connection* c) {
    // logging("Connected with: " + c->get_address());
  }
  void on_disconnected(connection* c) {
    // logging("Disconnected with: " + c->get_address());
  }
  void on_error(connection* c, connection::error e) {
    if (e == connection::no_error) {
      return;
    }
    LOG(ERROR) << "Error: " << std::to_string(e) << " (connection " << c->get_address() << ")";
  }
};

/// Acceptor handler
class lis_handler: public acceptor::acceptor_handler {
 public:
  node* node_;
  explicit lis_handler(node* n):node_(n) {}
  bool on_requested(const std::string& address) {
    // EXPECT_EQ(address, address_a);
    // logging("Connection request from: " + address + ". Accepting...");
    return true;
  }
  void on_connected(connection* c, const std::string& address) {
    // logging("Accepted connection from: " + address);
    c->async_read(new char[16], 16, 0, 0);
    node_->peers.push_back(c);
  }
  void on_error(connection::error e) {
    LOG(ERROR) << std::to_string(e);
  }
};

/// Function that collects and prints test results
void collect_stats() {
  heights_count.clear();
  // logging("Nodes size: " + std::to_string(nodes.size()));
  for (unsigned int i = 0; i < NUMBER_NODES; ++i) {
    heights_count[nodes[i]->height]++;
  }
  LOG(INFO) << "==== Heights ====";
  for (auto it = heights_count.begin(); it != heights_count.end(); ++it) {
    LOG(INFO) << std::to_string(it->first) << " -> " << std::to_string(it->second);
  }
  LOG(INFO) << "=================";
}

int main() {
  try {
    simulation* sim = simulation::get_simulator();
    LOG(INFO) << "Creating acceptors...";
    for (unsigned int i = 0; i < NUMBER_NODES; ++i) {
      nodes[i] = new node();
      nodes[i]->height = 0;
      nodes[i]->handler_ = new handler(nodes[i]);
      nodes[i]->acceptor_ =
          acceptor::create("sim", create_acceptor_address(i+1), new lis_handler(nodes[i]),
                          nodes[i]->handler_);
      nodes[i]->acceptor_->start_accepting();
    }
    LOG(INFO) << "Creating connections...";
    for (unsigned int i = 0; i < NUMBER_NODES; ++i) {
      for (unsigned int j = 0; j < NUMBER_PEERS_IN_NODE; ++j) {
        connection* new_connection = connection::create("sim",
                                                        create_connection_address(NUMBER_NODES, i),
                                                        nodes[i]->handler_);
        nodes[i]->peers.push_back(new_connection);
        new_connection->connect();
        new_connection->async_read(new char[16], 16, 0, 0);
      }
    }
    LOG(INFO) << "Starting simulation...";
    // ==============================================
    for (unsigned int i = 0; i < MAX_SIMULATION_TIME; i += LOOP_STEP) {
      if (i % PROCESS_STEP == 0) {
        LOG(INFO) << "PROCESSING: " + std::to_string(i);
        // sim->print_connections();
        int events_processed = sim->process(i);
        LOG(INFO) << "Events processed: " << events_processed;
        if ((i+LOOP_STEP) % BLOCK_CREATION_STEP == 0) {
          int n = std::rand() % NUMBER_NODES;
          ++nodes[n]->height;
          nodes[n]->send_height();
        }
      }
      collect_stats();
    }
    // sim->print_q();
  } catch (std::exception& e) {
    LOG(ERROR) << "EXCEPTION " + std::string(e.what());
  } catch(...) {
    LOG(ERROR) << "UNKOWN EXCEPTION!";
  }
  return 0;
}
