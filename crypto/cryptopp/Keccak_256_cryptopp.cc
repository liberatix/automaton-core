#include "crypto/cryptopp/Keccak_256_cryptopp.h"
#include "crypto/hash_transformation.h"
#include "cryptlib.h"  // NOLINT
#include "keccak.h"  // NOLINT

namespace automaton {
namespace core {
namespace crypto {

Keccak_256_cryptopp::Keccak_256_cryptopp() {
  hash = new CryptoPP::Keccak_256;
}

void Keccak_256_cryptopp::calculate_digest(const unsigned char * input,
                                           const size_t length,
                                           unsigned char * digest) {
  hash->CalculateDigest(digest, length == 0 ? nullptr : input, length);
}

void Keccak_256_cryptopp::update(const unsigned char * input,
                                 const size_t length) {
  hash->Update(length == 0 ? nullptr : input, length);
}

void Keccak_256_cryptopp::final(unsigned char * digest) {
  hash->Final(digest);
}

void Keccak_256_cryptopp::restart() {
  hash->Restart();
}

unsigned int Keccak_256_cryptopp::digest_size() const {
  return _digest_size;
}

bool Keccak_256_cryptopp::register_self() {
  hash_transformation::register_factory("Keccak_256",
    [] {return reinterpret_cast<hash_transformation*>
        (new Keccak_256_cryptopp()); });
  return true;
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
