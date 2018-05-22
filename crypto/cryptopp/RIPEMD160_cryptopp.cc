#include "crypto/cryptopp/RIPEMD160_cryptopp.h"
#include "crypto/hash_transformation.h"
#include "cryptlib.h"  // NOLINT
#include "ripemd.h"  // NOLINT

namespace automaton {
namespace core {
namespace crypto {

RIPEMD160_cryptopp::RIPEMD160_cryptopp() {
  hash = new CryptoPP::RIPEMD160;
}

void RIPEMD160_cryptopp::calculate_digest(const unsigned char * input,
                                          const size_t length,
                                          unsigned char * digest) {
  hash->CalculateDigest(digest, length == 0 ? nullptr : input, length);
}

void RIPEMD160_cryptopp::update(const unsigned char * input,
                                const size_t length) {
  hash->Update(length == 0 ? nullptr : input, length);
}

void RIPEMD160_cryptopp::final(unsigned char * digest) {
  hash->Final(digest);
}

void RIPEMD160_cryptopp::restart() {
  hash->Restart();
}

unsigned int RIPEMD160_cryptopp::digest_size() const {
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
