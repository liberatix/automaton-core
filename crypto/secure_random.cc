#include "crypto/secure_random.h"
#include <cryptlib.h>
#include <osrng.h>

bool secure_random::bit() {
  return prng.GenerateBit();
}

void secure_random::block(uint8_t * output, size_t size) {
  prng.GenerateBlock(output, size);
}

uint8_t secure_random::byte() {
  return prng.GenerateByte();
}
