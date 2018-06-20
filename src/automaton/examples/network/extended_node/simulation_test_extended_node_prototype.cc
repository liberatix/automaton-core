#include <cstdlib>
#include <iomanip>
#include <mutex>
#include <string>
#include <sstream>

#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/crypto/hash_transformation.h"
#include "automaton/core/log/log.h"
#include "automaton/core/network/simulated_connection.h"
#include "automaton/examples/crypto/basic_hash_miner.h"
#include "automaton/examples/network/extended_node/extended_node_prototype.h"


using automaton::core::crypto::hash_transformation;
using automaton::core::crypto::SHA256_cryptopp;
using automaton::core::network::acceptor;
using automaton::core::network::connection;
using automaton::core::network::simulation;
using automaton::examples::basic_hash_miner;
using automaton::examples::node;

std::mutex buffer_mutex;
std::vector<char*> buffers;

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
static const uint32_t BLOCK_CREATION_STEP = 1500;
static const uint32_t MAX_SIMULATION_TIME = 10000;

const int MINER_PRECISION_BITS = 14;

/// Global variables

/// height -> how many connections have that height
static std::map<std::string, uint32_t> hashes;
static std::map<std::string, uint32_t> heights;
static std::vector<node*> nodes(NUMBER_NODES);

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
std::string to_hex_string(uint8_t *data, int len) {
  const char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                          'a', 'b', 'c', 'd', 'e', 'f' };

  std::string s(len * 2, ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

static std::string tohex(std::string s) {
  std::stringstream ss;
  for (int i = 0; i < s.size(); i++) {
    ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') <<
        (static_cast<int32_t>(s[i]) & 0xff);
  }
  return ss.str();
}

/// Function that collects and prints test results
void collect_stats() {
  hashes.clear();
  heights.clear();
  // logging("Nodes size: " + std::to_string(nodes.size()));
  for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
    std::string hash = tohex(nodes[i]->chain_top);
    hashes[hash]++;
    if (heights.find(hash) != heights.end() && heights[hash] != nodes[i]->height) {
      LOG(ERROR) << "Hash doesn't match height!";
    }
    heights[hash] = nodes[i]->height;
  }
  LOG(INFO) << "==== Heights ====";
  for (auto it = hashes.begin(); it != hashes.end(); ++it) {
    LOG(INFO) << "HASH: " << it->first << " AT HEIGHT: " << heights[it->first] << " #PEERS: "
        << std::to_string(it->second);
  }
  LOG(INFO) << "=================";
}

int main() {
  SHA256_cryptopp::register_self();
  auto hash_transformation = hash_transformation::create("SHA256");
  basic_hash_miner miner(hash_transformation);
  try {
    simulation* sim = simulation::get_simulator();
    LOG(INFO) << "Creating acceptors...";
    for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
      nodes[i] = new node();
      nodes[i]->id = i;
      nodes[i]->init();
      nodes[i]->add_acceptor(i, "sim", create_acceptor_address(i+1));
    }
    LOG(INFO) << "Creating connections...";
    for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
      for (uint32_t j = 0; j < NUMBER_PEERS_IN_NODE; ++j) {
        nodes[i]->add_peer(nodes[i]->get_next_peer_id(), "sim",
            create_connection_address(NUMBER_NODES, i));
      }
    }
    LOG(INFO) << "Starting simulation...";
    // ==============================================
    std::string hash = "Zero block hash";
    int digest_size = hash_transformation->digest_size();
    for (uint32_t i = 0; i < MAX_SIMULATION_TIME; i += LOOP_STEP) {
      LOG(INFO) << "PROCESSING: " + std::to_string(i);
      // sim->print_connections();
      int events_processed = sim->process(i);
      LOG(INFO) << "Events processed: " << events_processed;
      // if (i < MAX_SIMULATION_TIME / 2 && (i+LOOP_STEP) % BLOCK_CREATION_STEP == 0) {
       if (i % BLOCK_CREATION_STEP == 0) {
        // =================== MINING =========================
        uint8_t* nonce = miner.mine(reinterpret_cast<const uint8_t*>(hash.c_str()),
            hash.size(), MINER_PRECISION_BITS);
        uint8_t* next_block_hash = new uint8_t[digest_size];
        hash_transformation->update(reinterpret_cast<const uint8_t*>(hash.c_str()), hash.size());
        hash_transformation->update(nonce, miner.get_nonce_lenght());
        hash_transformation->final(next_block_hash);
        hash = std::string(reinterpret_cast<char*>(next_block_hash), digest_size);
        LOG(INFO) << "HASH FOUND: " << to_hex_string(next_block_hash, digest_size);
        // =====================================================
        int n = std::rand()%NUMBER_NODES;
        nodes[n]->mine(hash);
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
