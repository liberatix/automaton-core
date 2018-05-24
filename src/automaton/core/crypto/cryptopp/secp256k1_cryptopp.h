#ifndef AUTOMATON_CORE_CRYPTO_CRYPTOPP_SECP256K1_CRYPTOPP_H_
#define AUTOMATON_CORE_CRYPTO_CRYPTOPP_SECP256K1_CRYPTOPP_H_

#include "automaton/core/crypto/digital_signature.h"
#include "eccrypto.h"  // NOLINT

namespace automaton {
namespace core {
namespace crypto {

class secp256k1_cryptopp : public digital_signature {
 public:
  size_t public_key_size();
  size_t private_key_size();
  size_t signature_size();
  size_t k_size();
  bool has_deterministic_signatures();

  // Input should be byte array encoding the integer,
  // each byte representing 2 4-byte values
  void gen_public_key(const unsigned char * private_key,
                      unsigned char * public_key);


  void sign(const unsigned char * private_key,
            const unsigned char * message,
            const size_t msg_len,
            unsigned char * signature);

  void sign_deterministic(const unsigned char * private_key,
                          const unsigned char * message,
                          const size_t msg_len,
                          const unsigned char * k,
                          unsigned char * signature);

  bool verify(const unsigned char * public_key,
              const unsigned char * message,
              const size_t msg_len,
              unsigned char * signature);

  static bool register_self();
};

}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  //  AUTOMATON_CORE_CRYPTO_CRYPTOPP_SECP256K1_CRYPTOPP_H_
