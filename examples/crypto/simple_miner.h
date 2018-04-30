#ifndef AUTOMATON_CORE_EXAMPLES_SIMPLE_MINER__
#define AUTOMATON_CORE_EXAMPLES_SIMPLE_MINER__

#include "crypto/hash_transformation.h"

class simple_miner {
 public:
  int get_nonce_lenght();
  explicit simple_miner(hash_transformation* hash_transformation);
  unsigned char* mine(unsigned char* block_hash,
    int block_hash_lenght,
    int required_leading_zeros);
};

#endif  // AUTOMATON_CORE_EXAMPLES_SIMPLE_MINER__
