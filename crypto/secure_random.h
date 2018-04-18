#ifndef AUTOMATON_CORE_CRYPTO_SECURE_RANDOM_CRYPTOPP_H__
#define AUTOMATON_CORE_CRYPTO_SECURE_RANDOM_CRYPTOPP_H__

#include <stdint.h>
#include "osrng.h" // NOLINT

// Class used for getting cryptographically secure random
class secure_random {
 public:
  // Generate random bit
  bool bit();
  // Generate random array of bytes.
  void block(uint8_t * buffer, size_t size);
  // Generate random array of byte
  uint8_t byte();
 private:
  CryptoPP::AutoSeededRandomPool prng;
};

#endif  // AUTOMATON_CORE_CRYPTO_SECURE_RANDOM_CRYPTOPP_H__
