#include <string>
#include "automaton/core/crypto/hash_transformation.h"
#include "automaton/core/crypto/cryptopp/SHA3_256_cryptopp.h"
#include "cryptlib.h"  // NOLINT
#include "sha.h"  // NOLINT
#include "gtest/gtest.h"  // NOLINT
#include "hex.h"  // NOLINT
#include "filters.h"  // NOLINT

using automaton::core::crypto::SHA3_256_cryptopp;
using automaton::core::crypto::hash_transformation;

// Helper function to convert bytes to hex values
// Each byte is converted to 2 hex values, encoding the left and
// right 4 bits of each byte.
static std::string toHex(uint8_t * digest, size_t size) {
  CryptoPP::HexEncoder encoder;
  std::string output;
  encoder.Attach(new CryptoPP::StringSink(output));
  encoder.Put(digest, size);
  encoder.MessageEnd();
  return output;
}

TEST(SHA3_256_cryptopp, calculate_digest) {
  SHA3_256_cryptopp hasher;
  size_t digest_size = hasher.digest_size();
  uint8_t* digest = new uint8_t[digest_size];
  constexpr uint32_t test_cases = 6;
  std::string long_a_string(1000000, 'a');

  std::string test[test_cases][2] = {
    {"a",
     "80084BF2FBA02475726FEB2CAB2D8215EAB14BC6BDD8BFB2C8151257032ECD8B"},
    {"abc",
     "3A985DA74FE225B2045C172D6BD390BD855F086E3E9D525B46BFE24511431532"},
    {"",
     "A7FFC6F8BF1ED76651C14756A061D662F580FF4DE43B49FA82D80A4B80F8434A"},
    {"testing",
     "7F5979FB78F082E8B1C676635DB8795C4AC6FABA03525FB708CB5FD68FD40C5E"},
    {"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"
     "ijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
     "916F6061FE879741CA6469B43971DFDB28B1A32DC36CB3254E812BE27AAD1D18"},
    {long_a_string,
      "5C8875AE474A3634BA4FD55EC85BFFD661F32ACA75C6D699D0CDCB6C115891C1"}
  };

  for (uint32_t i = 0; i < test_cases; i++) {
    hasher.calculate_digest(reinterpret_cast<const uint8_t*>(test[i][0].data()),
        test[i][0].length(), digest);
    EXPECT_EQ(toHex(digest, digest_size), test[i][1]);
  }

  delete[] digest;
}

TEST(SHA3_256_cryptopp, update_and_finish) {
  SHA3_256_cryptopp hasher;
  size_t digest_size = hasher.digest_size();
  uint8_t* digest = new uint8_t[digest_size];
  std::string test_input(
      "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno");
  const std::string EXP1 =
      "ECBBC42CBF296603ACB2C6BC0410EF4378BAFB24B710357F12DF607758B33E2B";
  const uint8_t* p_test_input = reinterpret_cast<const uint8_t*>(test_input.data());
  size_t len = test_input.length();

  for (uint32_t i = 0; i <  16777216; i++) {
    hasher.update(p_test_input, len);
  }
  hasher.final(digest);

  EXPECT_EQ(toHex(digest, digest_size), EXP1);

  // Try to hash a new string to see if everything restarted as intended
  const uint8_t* a = reinterpret_cast<const uint8_t*>("a");
  const uint8_t* b = reinterpret_cast<const uint8_t*>("b");
  const uint8_t* c = reinterpret_cast<const uint8_t*>("c");
  const std::string EXP2 =
      "3A985DA74FE225B2045C172D6BD390BD855F086E3E9D525B46BFE24511431532";
  hasher.update(a, 1);
  hasher.update(b, 1);
  hasher.update(c, 1);
  hasher.final(digest);

  EXPECT_EQ(toHex(digest, digest_size), EXP2);

  delete[] digest;
}

TEST(SHA3_256_cryptopp, digest_size) {
  SHA3_256_cryptopp hasher;
  EXPECT_EQ(hasher.digest_size(), CryptoPP::SHA3_256::DIGESTSIZE);
}
