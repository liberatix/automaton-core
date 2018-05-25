#include "automaton/core/crypto/cryptopp/RIPEMD160_cryptopp.h"
#include "automaton/core/crypto/hash_transformation.h"
#include "cryptlib.h"  // NOLINT
#include "ripemd.h"  // NOLINT

namespace automaton {
namespace core {
namespace crypto {

RIPEMD160_cryptopp::RIPEMD160_cryptopp() {
  hash = new CryptoPP::RIPEMD160;
}

void RIPEMD160_cryptopp::calculate_digest(const uint8_t * input,
                                          const size_t length,
                                          uint8_t * digest) {
  hash->CalculateDigest(digest, length == 0 ? nullptr : input, length);
}

void RIPEMD160_cryptopp::update(const uint8_t * input,
                                const size_t length) {
  hash->Update(length == 0 ? nullptr : input, length);
}

void RIPEMD160_cryptopp::final(uint8_t * digest) {
  hash->Final(digest);
}

void RIPEMD160_cryptopp::restart() {
  hash->Restart();
}

uint32_t RIPEMD160_cryptopp::digest_size() const {
  return _digest_size;
}

bool RIPEMD160_cryptopp::register_self() {
  hash_transformation::register_factory("RIPEMD160",
  [] {return reinterpret_cast<hash_transformation*>
      (new RIPEMD160_cryptopp()); });
  return true;
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
