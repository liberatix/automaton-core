#include "crypto/SHA256_cryptopp.h"
#include "crypto/hash_transformation.h"
#include "cryptlib.h"  // NOLINT
#include "sha.h"  // NOLINT

SHA256_cryptopp::SHA256_cryptopp() {
  hash = new CryptoPP::SHA256;
}

void SHA256_cryptopp::calculate_digest(const unsigned char * input,
                                      const size_t length,
                                      unsigned char * digest) {
  if (length) {
    hash->CalculateDigest(digest, input, length);
  } else {
    hash->Final(digest);
  }
}

void SHA256_cryptopp::update(const unsigned char * input,
                             const size_t length) {
  hash->Update(input, length);
}

void SHA256_cryptopp::final(unsigned char * digest) {
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
