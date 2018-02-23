#include "SHA256_cryptopp.h"
#include "hash_transformation.h"

#include "cryptlib.h"
#include "sha.h"

SHA256_cryptopp::SHA256_cryptopp() {
  hash = new CryptoPP::SHA256;
}

void SHA256_cryptopp::calculate_digest(const unsigned char * input, const size_t length, unsigned char * digest) {
  hash->CalculateDigest(digest, input, length);
}

void SHA256_cryptopp::update(const unsigned char * input, const size_t length) {
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

bool SHA256_cryptopp::registerSelf() {
  //  hash_transformation::register_factory("SHA256", [] {return (hash_transformation*)new SHA256_cryptopp(); });
  return true;
}

static bool registration = SHA256_cryptopp::registerSelf();
