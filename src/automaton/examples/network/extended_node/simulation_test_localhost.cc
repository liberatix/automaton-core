#include <cstdlib>
#include <iomanip>
#include <mutex>
#include <string>
#include <sstream>

#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/crypto/hash_transformation.h"
#include "automaton/core/io/io.h"
#include "automaton/core/log/log.h"
#include "automaton/core/network/tcp_implementation.h"
#include "automaton/examples/network/extended_node/extended_node_prototype.h"

using automaton::core::crypto::hash_transformation;
using automaton::core::crypto::SHA256_cryptopp;
using automaton::core::network::acceptor;
using automaton::core::network::connection;
using automaton::examples::node;

/// Constants

static const int FIRST_ACCEPTOR_PORT = 11100;
static const uint32_t NUMBER_NODES = 100;
// These include only the peers that a node connects to, not the accepted ones
static const uint32_t NUMBER_PEERS_IN_NODE = 2;
static const uint32_t MIN_CONNECTIONS = 0;
static const uint32_t MAX_CONNECTIONS = 1;
static const uint32_t LOOP_STEP = 48;
static const uint32_t SIMULATION_TIME = 10000;
static const uint32_t MINER_PRECISION_BITS = 20;
static const uint32_t NEW_NODES = 10;

static const char* LOCALHOST = "127.0.0.1:";

/// Global variables

/// height -> how many connections have that height
static std::map<std::string, uint32_t> hashes;
static std::map<std::string, uint32_t> heights;
static std::vector<node*> nodes;
std::mutex nodes_mutex;
bool simulation_end = false;
std::thread miner;

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
  // LOG(DEBUG) << "stats 0";
  // LOG(DEBUG) << "stats: waiting for nodes mutex 0";
  nodes_mutex.lock();
  // LOG(DEBUG) << "stats: nodes mutex locked 0";
  // logging("Nodes size: " + std::to_string(nodes.size()));
  for (uint32_t i = 0; i < nodes.size(); ++i) {
    // LOG(DEBUG) << i << " stats: 0";
    auto res = nodes[i]->get_height_and_top();
    std::string hash = automaton::core::io::string_to_hex(res.second);
    hashes[hash]++;
    heights[hash] = res.first;
    // LOG(DEBUG) << i << " stats: 1";
  }
  // LOG(DEBUG) << "stats: nodes mutex unlocked 1";
  LOG(INFO) << "==== Heights ====";
  for (auto it = hashes.begin(); it != hashes.end(); ++it) {
    LOG(INFO) << "HASH: " << it->first << " AT HEIGHT: " << heights[it->first] << " #PEERS: "
        << std::to_string(it->second);
  }
  LOG(INFO) << "=================";
  nodes_mutex.unlock();
}

void miner_thread_function() {
  try {
    while (!simulation_end) {
      // LOG(DEBUG) << "mine 0";
      // LOG(DEBUG) << "mine: waiting for nodes mutex 0";
      nodes_mutex.lock();
      // LOG(DEBUG) << "mine: nodes mutex locked 0";
      uint32_t n_number = nodes.size();
      nodes_mutex.unlock();
      // LOG(DEBUG) << "mine: nodes mutex unlocked 0";
      for (uint32_t i = 0; i < n_number && !simulation_end; ++i) {
        // LOG(DEBUG) << i << " mine: waiting for nodes mutex 1";
        nodes_mutex.lock();
        // LOG(DEBUG) << i << "of (" << n_number << ") {"<< nodes.size() <<
            // "} mine: nodes mutex locked 1";
        node* n = nodes[i];
        nodes_mutex.unlock();
        // // LOG(DEBUG) << i << " mine: nodes mutex unlocked 1";
        // LOG(DEBUG) << i << " before mine";
        n->mine(128, MINER_PRECISION_BITS);
        // nodes_mutex.unlock();
        // LOG(DEBUG) << i << " after mine";
      }
      // nodes_mutex.unlock();
      // LOG(DEBUG) << "mine 2";
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "EXCEPTION " + std::string(e.what());
  } catch(...) {
    LOG(ERROR) << "UNKOWN EXCEPTION!";
  }
}

int main() {
  try {
    automaton::core::network::tcp_init();
    LOG(INFO) << "Creating acceptors...";
    for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
      std::string address = LOCALHOST + std::to_string(FIRST_ACCEPTOR_PORT + i);
      nodes.push_back(new node());
      nodes[i]->init();
      nodes[i]->id = address;
      nodes[i]->add_acceptor(address, "tcp", address);
    }
    LOG(INFO) << "Creating connections...";
    for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
      for (uint32_t j = 0; j < NUMBER_PEERS_IN_NODE; ++j) {
        std::string address;
        do {
          address = LOCALHOST +
              std::to_string(FIRST_ACCEPTOR_PORT + std::rand() % NUMBER_NODES);
        } while (address == nodes[i]->id);
        nodes[i]->add_peer(address, "tcp", address);
      }
    }
    LOG(INFO) << "Starting simulation...";
    miner = std::thread(miner_thread_function);
    // ==============================================
    for (uint32_t i = 0; i < SIMULATION_TIME / 3; i += LOOP_STEP) {
      LOG(INFO) << "PROCESSING: " + std::to_string(i);
      collect_stats();
      std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_STEP));
    }
    nodes_mutex.lock();
    for (uint32_t i = NUMBER_NODES; i < NUMBER_NODES + NEW_NODES; ++i) {
      std::string address = LOCALHOST + std::to_string(FIRST_ACCEPTOR_PORT + i);
      nodes.push_back(new node());
      nodes[i]->init();
      nodes[i]->id = address;
      nodes[i]->add_acceptor(address, "tcp", address);
    }
    LOG(INFO) << "Creating connections...";
    for (uint32_t i = NUMBER_NODES; i < NUMBER_NODES + NEW_NODES; ++i) {
      for (uint32_t j = 0; j < NUMBER_PEERS_IN_NODE; ++j) {
        std::string address;
        do {
          address = LOCALHOST +
              std::to_string(FIRST_ACCEPTOR_PORT + std::rand() % NUMBER_NODES);
        } while (address == nodes[i]->id);
        nodes[i]->add_peer(address, "tcp", address);
      }
    }
    nodes_mutex.unlock();
    LOG(INFO) << "Continuing simulation...";
    for (uint32_t i = SIMULATION_TIME / 3; i < SIMULATION_TIME; i += LOOP_STEP) {
      LOG(INFO) << "PROCESSING: " + std::to_string(i);
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
  collect_stats();
  for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
    delete nodes[i];
  }
  return 0;
}
