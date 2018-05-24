#ifndef AUTOMATON_CORE_CRYPTO_CRYPTOPP_SECURE_RANDOM_CRYPTOPP_H_
#define AUTOMATON_CORE_CRYPTO_CRYPTOPP_SECURE_RANDOM_CRYPTOPP_H_

#include <stdint.h>
#include <osrng.h>
#include "automaton/core/crypto/secure_random.h"

namespace automaton {
namespace core {
namespace crypto {

// Class used for getting cryptographically secure random
class secure_random_cryptopp : public secure_random {
 public:
  bool bit();

  void block(uint8_t * buffer, size_t size);

  uint8_t byte();

  static bool register_self();
 private:
  CryptoPP::AutoSeededRandomPool prng;
};

}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_CRYPTO_CRYPTOPP_SECURE_RANDOM_CRYPTOPP_H_