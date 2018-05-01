#include "crypto/secure_random_cryptopp.h"
#include <cryptlib.h>
#include <osrng.h>

bool secure_random_cryptopp::bit() {
  return prng.GenerateBit();
}

void secure_random_cryptopp::block(uint8_t * output, size_t size) {
  prng.GenerateBlock(output, size);
}

uint8_t secure_random_cryptopp::byte() {
  return prng.GenerateByte();
}

bool secure_random_cryptopp::register_self() {
  secure_random_cryptopp::register_factory("cryptopp", [] {return
      reinterpret_cast<secure_random*>(new secure_random_cryptopp()); });
  return true;
}
