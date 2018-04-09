  #ifndef AUTOMATON_CORE_CRYPTO_DSA_CRYPTOPP_IMPL_TRANSFORMATION_H__
#define AUTOMATON_CORE_CRYPTO_DSA_CRYPTOPP_IMPL_TRANSFORMATION_H__

#include "crypto/dsa.h"
#include "eccrypto.h"  // NOLINT

class dsa_cryptopp : public dsa {
 public:
  size_t public_key_size();
  size_t private_key_size();
  // will delete hashed_message_size, we need to give the size
  // of the message depending on the hash functions
  size_t hashed_message_size();
  size_t signature_size();
  size_t k_size();
  bool has_deterministic_signatures();

  // Input should be byte array encoding the integer,
  // each byte representing 2 4-byte values
  void gen_public_key(const unsigned char * private_key,
                      unsigned char * public_key);


  void sign(const unsigned char * private_key,
            const unsigned char * message,
            unsigned char * signature);

  void sign_deterministic(const unsigned char * private_key,
                          const unsigned char * message,
                          const unsigned char * k,
                          unsigned char * signature);

  bool verify(const unsigned char * public_key,
              const unsigned char * message,
              unsigned char * signature);

  static bool register_self();
};

#endif  //  AUTOMATON_CORE_CRYPTO_DSA_CRYPTOPP_IMPL_TRANSFORMATION_H__
