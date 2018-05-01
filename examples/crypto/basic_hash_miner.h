#ifndef AUTOMATON_CORE_EXAMPLES_BASIC_HASH_MINER__
#define AUTOMATON_CORE_EXAMPLES_BASIC_HASH_MINER__

#include "crypto/hash_transformation.h"

class basic_hash_miner {
 public:
  int get_nonce_lenght();
  explicit basic_hash_miner(hash_transformation* hash_transformation);
  unsigned char* mine(unsigned char* block_hash,
    int block_hash_lenght,
    int required_leading_zeros);
};

#endif  // AUTOMATON_CORE_EXAMPLES_SIMPLE_MINER__
