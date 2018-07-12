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

// Constants

// static const uint32_t NUMBER_NODES = 64;
static const uint32_t NUMBER_PEERS_IN_NODE = 2;
static const uint32_t LOOP_STEP = 48;
static const uint32_t SIMULATION_TIME = 10000;
static const uint32_t MINER_PRECISION_BITS = 20;
static const uint32_t NEW_NODES = 48;

static const char* LOCALHOST = "127.0.0.1:";

// Global variables

static uint32_t MIN_PORT = 0;
static uint32_t MAX_PORT = 0;
static uint32_t MY_MIN_PORT = 0;
static uint32_t MY_MAX_PORT = 0;
static uint32_t NUMBER_NODES = 0;

// height -> how many connections have that height
static std::map<std::string, uint32_t> hashes;
static std::map<std::string, uint32_t> heights;
static std::vector<node*> nodes;
std::mutex nodes_mutex;
bool simulation_end = false;
std::thread miner;
std::thread updater;

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

// Function that collects and prints test results
void collect_stats() {
  hashes.clear();
  heights.clear();
  nodes_mutex.lock();
  for (uint32_t i = 0; i < nodes.size(); ++i) {
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
  nodes_mutex.unlock();
}

void update_thread_function() {
  try {
    while (!simulation_end) {
      nodes_mutex.lock();
      uint32_t n_number = nodes.size();
      nodes_mutex.unlock();
      for (uint32_t i = 0; i < n_number && !simulation_end; ++i) {
        nodes_mutex.lock();
        node* n = nodes[i];
        nodes_mutex.unlock();
        n->update();
      }
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "EXCEPTION " + std::string(e.what());
  } catch(...) {
    LOG(ERROR) << "UNKOWN EXCEPTION!";
  }
}

void miner_thread_function() {
  try {
    while (!simulation_end) {
      nodes_mutex.lock();
      uint32_t n_number = nodes.size();
      nodes_mutex.unlock();
      for (uint32_t i = 0; i < n_number && !simulation_end; ++i) {
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

int main(int argc, const char * argv[]) {
  if (argc != 5) {
    LOG(ERROR) << "Number of provided arguments is not as expected!";
    return 0;
  }
  try {
    MIN_PORT = std::stoi(argv[1]);
    MAX_PORT = std::stoi(argv[2]);
    MY_MIN_PORT = std::stoi(argv[3]);
    MY_MAX_PORT = std::stoi(argv[4]);
    NUMBER_NODES = MY_MAX_PORT - MY_MIN_PORT + 1;
    automaton::core::network::tcp_init();
    node::node_params params;
    params.acceptors_count = 1;
    params.connected_peers_count = NUMBER_PEERS_IN_NODE;
    params.min_port_number = MIN_PORT;
    params.max_port_number = MAX_PORT;
    LOG(INFO) << "Creating acceptors...";
    for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
      std::string address = LOCALHOST + std::to_string(MY_MIN_PORT + i);
      nodes.push_back(new node(params));
      nodes[i]->init();
      nodes[i]->add_acceptor("tcp", address);
    }
    LOG(INFO) << "Starting simulation...";
    updater = std::thread(update_thread_function);
    miner = std::thread(miner_thread_function);
    // ==============================================
    for (uint32_t i = 0; i < SIMULATION_TIME; i += LOOP_STEP) {
      // LOG(INFO) << "PROCESSING: " + std::to_string(i);
      // collect_stats();
      std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_STEP));
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "EXCEPTION " + std::string(e.what());
  } catch(...) {
    LOG(ERROR) << "UNKOWN EXCEPTION!";
  }
  simulation_end = true;
  miner.join();
  updater.join();
  collect_stats();
  automaton::core::network::tcp_release();
  for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
    delete nodes[i];
  }
  return 0;
}
