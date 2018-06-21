#include <cstdlib>
#include <iomanip>
#include <mutex>
#include <string>
#include <sstream>

#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/crypto/hash_transformation.h"
#include "automaton/core/log/log.h"
#include "automaton/core/network/tcp_implementation.h"
#include "automaton/examples/crypto/basic_hash_miner.h"
#include "automaton/examples/network/extended_node/extended_node_prototype.h"

using automaton::core::crypto::hash_transformation;
using automaton::core::crypto::SHA256_cryptopp;
using automaton::core::network::acceptor;
using automaton::core::network::connection;
using automaton::examples::basic_hash_miner;
using automaton::examples::node;

std::mutex buffer_mutex;

/// Constants

static const int FIRST_ACCEPTOR_PORT = 12345;
static const uint32_t NUMBER_NODES = 3000;
// These include only the peers that a node connects to, not the accepted ones
static const uint32_t NUMBER_PEERS_IN_NODE = 2;
static const uint32_t MIN_LAG = 100;
static const uint32_t MAX_LAG = 1000;
static const uint32_t MIN_CONNECTIONS = 0;
static const uint32_t MAX_CONNECTIONS = 1;
static const uint32_t MIN_BANDWIDTH = 16;
static const uint32_t MAX_BANDWIDTH = 16;
static const uint32_t LOOP_STEP = 100;
static const uint32_t BLOCK_CREATION_STEP = 1500;
static const uint32_t MAX_SIMULATION_TIME = 10000;
const uint32_t MINER_PRECISION_BITS = 10;

const char* LOCALHOST = "127.0.0.1:";

/// Global variables

/// height -> how many connections have that height
static std::map<std::string, uint32_t> hashes;
static std::map<std::string, uint32_t> heights;
static std::vector<node*> nodes(NUMBER_NODES);

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

static std::string tohex(std::string s) {
  std::stringstream ss;
  for (uint32_t i = 0; i < s.size(); i++) {
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
    auto res = nodes[i]->get_height_and_top();
    std::string hash = tohex(res.second);
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

int main() {
  try {
    SHA256_cryptopp::register_self();
    auto hash_transformation = hash_transformation::create("SHA256");
    basic_hash_miner miner(hash_transformation);
    automaton::core::network::tcp_init();
    LOG(INFO) << "Creating acceptors...";
    for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
      nodes[i] = new node();
      nodes[i]->init();
      nodes[i]->id = i;
      nodes[i]->add_acceptor(i, "tcp", LOCALHOST + std::to_string(FIRST_ACCEPTOR_PORT + i));
    }
    LOG(INFO) << "Creating connections...";
    for (uint32_t i = 0; i < NUMBER_NODES; ++i) {
      for (uint32_t j = 0; j < NUMBER_PEERS_IN_NODE; ++j) {
        nodes[i]->add_peer(nodes[i]->get_next_peer_id(), "tcp", LOCALHOST +
            std::to_string(FIRST_ACCEPTOR_PORT + std::rand() % NUMBER_NODES));
      }
    }
    LOG(INFO) << "Starting simulation...";
    // ==============================================
    std::string hash = "Zero block hash";
    int32_t digest_size = hash_transformation->digest_size();
    for (uint32_t i = 0; i < MAX_SIMULATION_TIME; i += LOOP_STEP) {
      LOG(INFO) << "PROCESSING: " + std::to_string(i);
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
        uint32_t n = std::rand()%NUMBER_NODES;
        nodes[n]->mine(hash);
      }
      collect_stats();
      std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_STEP));
    }
    // sim->print_q();
  } catch (std::exception& e) {
    LOG(ERROR) << "EXCEPTION " + std::string(e.what());
  } catch(...) {
    LOG(ERROR) << "UNKOWN EXCEPTION!";
  }
  return 0;
}
