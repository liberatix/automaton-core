#ifndef AUTOMATON_CORE_CRYPTO_RIPEMD160_CRYPTOPP_H__
#define AUTOMATON_CORE_CRYPTO_RIPEMD160_CRYPTOPP_H__

#include "crypto/hash_transformation.h"

namespace CryptoPP {
class RIPEMD160;
}

class RIPEMD160_cryptopp : public hash_transformation {
 private:
  CryptoPP::RIPEMD160* hash;
 public:
  RIPEMD160_cryptopp();

  void calculate_digest(const unsigned char* input,
                        const size_t length,
                        unsigned char* digest);

  void update(const unsigned char* input, const size_t length);

  void final(unsigned char* digest);

  void restart();

  unsigned int digest_size() const;

  static bool register_self();

 private:
  static const int _digest_size = 20;
};

#endif  // AUTOMATON_CORE_CRYPTO_RIPEMD160_CRYPTOPP_H__
