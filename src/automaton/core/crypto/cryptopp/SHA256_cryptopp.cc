#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "automaton/core/crypto/hash_transformation.h"
#include "cryptlib.h"  // NOLINT
#include "sha.h"  // NOLINT

namespace automaton {
namespace core {
namespace crypto {

SHA256_cryptopp::SHA256_cryptopp() {
  hash = new CryptoPP::SHA256;
}

void SHA256_cryptopp::calculate_digest(const uint8_t * input,
                                      const size_t length,
                                      uint8_t * digest) {
  hash->CalculateDigest(digest, length == 0 ? nullptr : input, length);
}

void SHA256_cryptopp::update(const uint8_t * input,
                             const size_t length) {
  hash->Update(length == 0 ? nullptr : input, length);
}

void SHA256_cryptopp::final(uint8_t * digest) {
  hash->Final(digest);
}

void SHA256_cryptopp::restart() {
  hash->Restart();
}

unsigned int SHA256_cryptopp::digest_size() const {
  return _digest_size;
}

bool SHA256_cryptopp::register_self() {
  hash_transformation::register_factory("SHA256",
  [] {return reinterpret_cast<hash_transformation*>(new SHA256_cryptopp()); });
  return true;
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
