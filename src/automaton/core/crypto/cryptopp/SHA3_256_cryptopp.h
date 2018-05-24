#ifndef AUTOMATON_CORE_CRYPTO_CRYPTOPP_SHA3_256_CRYPTOPP_H_
#define AUTOMATON_CORE_CRYPTO_CRYPTOPP_SHA3_256_CRYPTOPP_H_

#include "automaton/core/crypto/hash_transformation.h"
#include "sha3.h"  // NOLINT

namespace automaton {
namespace core {
namespace crypto {

class SHA3_256_cryptopp : public hash_transformation {
 private:
  CryptoPP::SHA3_256* hash;
 public:
  SHA3_256_cryptopp();

  void calculate_digest(const unsigned char* input,
                        const size_t length,
                        unsigned char* digest);

  void update(const unsigned char* input, const size_t length);

  void final(unsigned char* digest);

  void restart();

  unsigned int digest_size() const;

  static bool register_self();

 private:
  static const int _digest_size = 32;
};

}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_CRYPTO_CRYPTOPP_SHA3_256_CRYPTOPP_H_
