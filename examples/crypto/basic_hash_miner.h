#ifndef AUTOMATON_CORE_EXAMPLES_BASIC_HASH_MINER__
#define AUTOMATON_CORE_EXAMPLES_BASIC_HASH_MINER__

#include "crypto/hash_transformation.h"

class basic_hash_miner {
  public:
    explicit basic_hash_miner(hash_transformation* hash_transformation);

    int get_nonce_lenght();
    unsigned char* mine(unsigned char* block_hash,
      int block_hash_lenght,
      int required_leading_zeros);

  private:
    int nonce_lenght_;
    hash_transformation* hash_transformation_;
    unsigned char* nonce_;

    void next_nonce();
    bool is_valid_next_block_hash(unsigned char* hash, int required_leading_zeros);
};

#endif  // AUTOMATON_CORE_EXAMPLES_SIMPLE_MINER__
