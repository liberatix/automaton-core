#include "crypto/cryptopp/SHA512_cryptopp.h"
#include "crypto/hash_transformation.h"
#include "cryptlib.h"  // NOLINT
#include "sha.h"  // NOLINT

SHA512_cryptopp::SHA512_cryptopp() {
  hash = new CryptoPP::SHA512;
}

void SHA512_cryptopp::calculate_digest(const unsigned char * input,
                                      const size_t length,
                                      unsigned char * digest) {
  hash->CalculateDigest(digest, length == 0 ? nullptr : input, length);
}

void SHA512_cryptopp::update(const unsigned char * input,
                             const size_t length) {
  hash->Update(length == 0 ? nullptr : input, length);
}

void SHA512_cryptopp::final(unsigned char * digest) {
  hash->Final(digest);
}

void SHA512_cryptopp::restart() {
  hash->Restart();
}

unsigned int SHA512_cryptopp::digest_size() const {
  return _digest_size;
}

bool SHA512_cryptopp::register_self() {
  hash_transformation::register_factory("SHA512",
  [] {return reinterpret_cast<hash_transformation*>(new SHA512_cryptopp()); });
  return true;
}
