#ifndef AUTOMATON_CORE_CRYPTO_KECCAK_256_CRYPTOPP_H__
#define AUTOMATON_CORE_CRYPTO_KECCAK_256_CRYPTOPP_H__

#include "crypto/hash_transformation.h"
#include "cryptlib.h"  // NOLINT
#include "keccak.h"  // NOLINT

class Keccak_256_cryptopp : public hash_transformation {
private:
  CryptoPP::Keccak_256 * hash;
public:
  Keccak_256_cryptopp();

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

#endif  // AUTOMATON_CORE_CRYPTO_Keccak_256_CRYPTOPP_H__
