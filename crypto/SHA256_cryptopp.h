#ifndef AUTOMATON_CORE_CRYPTO_SHA256_CRYPTOPP_H__
#define AUTOMATON_CORE_CRYPTO_SHA256_CRYPTOPP_H__

#include "crypto/hash_transformation.h"

namespace CryptoPP {
class SHA256;
}

class SHA256_cryptopp : public hash_transformation {
 private:
  CryptoPP::SHA256* hash;
 public:
  SHA256_cryptopp();

  void calculate_digest(const unsigned char* input,
                        const size_t length,
                        unsigned char* digest);

  void update(const unsigned char* input, const size_t length);

  void final(unsigned char* digest);

  void restart();

  unsigned int digest_size() const;

  static bool registerSelf();

 private:
  static const int _digest_size = 32;
};

#endif  // AUTOMATON_CORE_CRYPTO_SHA256_CRYPTOPP_H__
