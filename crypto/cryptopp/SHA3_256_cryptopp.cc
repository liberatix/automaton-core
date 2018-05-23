#include "crypto/cryptopp/SHA3_256_cryptopp.h"
#include "crypto/hash_transformation.h"
#include "cryptlib.h"  // NOLINT
#include "sha3.h"  // NOLINT

namespace automaton {
namespace core {
namespace crypto {

SHA3_256_cryptopp::SHA3_256_cryptopp() {
  hash = new CryptoPP::SHA3_256;
}

void SHA3_256_cryptopp::calculate_digest(const unsigned char * input,
                                      const size_t length,
                                      unsigned char * digest) {
  hash->CalculateDigest(digest, length == 0 ? nullptr : input, length);
}

void SHA3_256_cryptopp::update(const unsigned char * input,
                             const size_t length) {
  hash->Update(length == 0 ? nullptr : input, length);
}

void SHA3_256_cryptopp::final(unsigned char * digest) {
  hash->Final(digest);
}

void SHA3_256_cryptopp::restart() {
  hash->Restart();
}

unsigned int SHA3_256_cryptopp::digest_size() const {
  return _digest_size;
}

bool SHA3_256_cryptopp::register_self() {
  hash_transformation::register_factory("SHA3_256",
  [] {return reinterpret_cast<hash_transformation*>
      (new SHA3_256_cryptopp()); });
  return true;
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
