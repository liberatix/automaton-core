#ifndef AUTOMATON_EXAMPLES_CRYPTO_BASIC_HASH_MINER_H_
#define AUTOMATON_EXAMPLES_CRYPTO_BASIC_HASH_MINER_H_

#include "automaton/core/crypto/hash_transformation.h"

namespace automaton {
namespace core {
namespace examples {

class basic_hash_miner {
 public:
  explicit basic_hash_miner(crypto::hash_transformation* hash_transformation);

  int get_nonce_lenght();
  unsigned char* mine(unsigned char* block_hash,
                      int block_hash_lenght,
                      int required_leading_zeros);

 private:
  int nonce_lenght_;
  crypto::hash_transformation* hash_transformation_;
  unsigned char* nonce_;

  void next_nonce();
  bool is_valid_next_block_hash(unsigned char* hash,
                                int required_leading_zeros);
};

}  // namespace examples
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_EXAMPLES_CRYPTO_BASIC_HASH_MINER_H_
