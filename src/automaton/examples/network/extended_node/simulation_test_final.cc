#include <cstdlib>
#include <iomanip>
#include <mutex>
#include <string>
#include <sstream>

#include "automaton/core/io/io.h"
#include "automaton/core/log/log.h"
#include "automaton/core/network/simulated_connection.h"
#include "automaton/examples/network/extended_node/extended_node_prototype.h"

using automaton::core::network::acceptor;
using automaton::core::network::connection;
using automaton::core::network::simulation;
using automaton::examples::node;

/// Constants

static const uint32_t NUMBER_NODES = 1000;
// These include only the peers that a node connects to, not the accepted ones
static const uint32_t NUMBER_PEERS_IN_NODE = 4;
static const uint32_t MIN_LAG = 100;
static const uint32_t MAX_LAG = 1000;
static const uint32_t MIN_CONNECTIONS = 0;
static const uint32_t MAX_CONNECTIONS = 1;
static const uint32_t MIN_BANDWIDTH = 512;
static const uint32_t MAX_BANDWIDTH = 512;
static const uint32_t LOOP_STEP = 100;
static const uint32_t SIMULATION_TIME = 10000;

const uint32_t MINER_PRECISION_BITS = 20;

/// Global variables

/// height -> how many connections have that height
static std::map<std::string, uint32_t> hashes;
static std::map<std::string, uint32_t> heights;
static std::vector<node*> nodes(NUMBER_NODES);
std::mutex nodes_mutex;
bool simulation_end = false;
std::thread miner;

/// Helper functions for creating addresses

std::string create_connection_address(uint32_t num_acceptors, uint32_t this_acceptor,
                                      uint32_t min_lag, uint32_t max_lag,
                                      uint32_t min_bandwidth, uint32_t max_bandwidth) {
  /// Choosing random min (mn) and max lag (mx): min_lag <= mn < mx <= max_lag
  std::stringstream s;
  uint32_t mn, mx, acc;
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
  s << (std::rand() % (max_bandwidth - min_bandwidth + 1) + min_bandwidth) << ':' << acc;
  // logging("Created connection address: " + s.str());
  return s.str();
}
std::string create_connection_address(uint32_t num_acceptors, uint32_t this_acceptor) {
  return create_connection_address(num_acceptors, this_acceptor, MIN_LAG, MAX_LAG,
                                  MIN_BANDWIDTH, MAX_BANDWIDTH);
}
std::string create_acceptor_address(uint32_t address,
                                    uint32_t min_connections, uint32_t max_connections,
                                    uint32_t min_bandwidth, uint32_t max_bandwidth) {
  std::stringstream s;
  uint32_t conns;
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
std::string to_hex_string(uint8_t *data, uint32_t len) {
  const char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                          'a', 'b', 'c', 'd', 'e', 'f' };

  std::string s(len * 2, ' ');
  for (uint32_t i = 0; i < len; ++i) {
    s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

/// Function that collects and prints test results
void collect_stats() {
  hashes.clear();
  heights.clear();
  // logging("Nodes size: " + std::to_string(nodes.size()));
  for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
    auto res = nodes[i]->get_height_and_top();
    std::string hash = automaton::core::io::string_to_hex(res.second);
    hashes[hash]++;
    heights[hash] = res.first;
  }
  LOG(INFO) << "==== Heights ====";
  for (auto it = hashes.begin(); it != hashes.end(); ++it) {
    LOG(INFO) << "HASH: " << it->first << " AT HEIGHT: " << heights[it->first] << " #PEERS: "
        << std::to_string(it->second);
  }
  LOG(INFO) << "=================";
}

void miner_thread_function() {
  try {
    while (!simulation_end) {
      for (uint32_t i = 0; i < NUMBER_NODES && !simulation_end; ++i) {
        nodes_mutex.lock();
        node* n = nodes[i];
        nodes_mutex.unlock();
        n->mine(128, MINER_PRECISION_BITS);
      }
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "EXCEPTION " + std::string(e.what());
  } catch(...) {
    LOG(ERROR) << "UNKOWN EXCEPTION!";
  }
}

int main() {
  try {
    simulation* sim = simulation::get_simulator();
    LOG(INFO) << "Creating acceptors...";
    for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
      nodes[i] = new node();
      nodes[i]->id = std::to_string(i);
      nodes[i]->init();
      nodes[i]->add_acceptor(std::to_string(i), "sim", create_acceptor_address(i+1));
    }
    LOG(INFO) << "Creating connections...";
    for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
      for (uint32_t j = 0; j < NUMBER_PEERS_IN_NODE; ++j) {
        nodes[i]->add_peer(std::to_string(nodes[i]->get_next_peer_id()), "sim",
            create_connection_address(NUMBER_NODES, i));
      }
    }
    LOG(INFO) << "Starting simulation...";
    miner = std::thread(miner_thread_function);
    // ==============================================
    for (uint32_t i = 0; i < SIMULATION_TIME; i += LOOP_STEP) {
      LOG(INFO) << "PROCESSING: " + std::to_string(i);
      int32_t events_processed = sim->process(i);
      LOG(INFO) << "Events processed: " << events_processed;
      collect_stats();
      std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_STEP));
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "EXCEPTION " + std::string(e.what());
  } catch(...) {
    LOG(ERROR) << "UNKOWN EXCEPTION!";
  }
  simulation_end = true;
  miner.join();
  for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
    delete nodes[i];
  }
  return 0;
}
