#include <string>
#include "crypto/hash_transformation.h"
#include "crypto/Keccak_256_cryptopp.h"
#include "cryptlib.h"  // NOLINT
#include "keccak.h"  // NOLINT
#include "gtest/gtest.h"  // NOLINT
#include "hex.h"  // NOLINT
#include "filters.h"  // NOLINT

// Helper function to convert bytes to hex values
// Each byte is converted to 2 hex values, encoding the left and
// right 4 bits of each byte.
static std::string toHex(unsigned char * digest, size_t size) {
  CryptoPP::HexEncoder encoder;
  std::string output;
  encoder.Attach(new CryptoPP::StringSink(output));
  encoder.Put(digest, size);
  encoder.MessageEnd();
  return output;
}

TEST(keccak_256_cryptopp, register_self) {
  Keccak_256_cryptopp::register_self();
  hash_transformation * hasher = hash_transformation::create("Keccak_256");
  EXPECT_NE(hasher, nullptr);
}

TEST(keccak_256_cryptopp, calculate_digest) {
  Keccak_256_cryptopp hasher;
  size_t digest_size = hasher.digest_size();
  unsigned char* digest = new unsigned char[digest_size];
  constexpr unsigned int test_cases = 6;

  std::string test[test_cases][2] = {
    {"a",
     "3AC225168DF54212A25C1C01FD35BEBFEA408FDAC2E31DDD6F80A4BBF9A5F1CB"},
    {"abc",
     "4E03657AEA45A94FC7D47BA826C8D667C0D1E6E33A64A036EC44F58FA12D6C45"},
    {"",
     "C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470"},
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
     "45D3B367A6904E6E8D502EE04999A7C27647F91FA845D456525FD352AE3D7371"},
    {"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"
     "ijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
     "F519747ED599024F3882238E5AB43960132572B7345FBEB9A90769DAFD21AD67"},
    {"testing",
      "5F16F4C7F149AC4F9510D9CF8CF384038AD348B3BCDC01915F95DE12DF9D1B02"}
  };

  for (unsigned int i = 0; i < test_cases; i++) {
    hasher.calculate_digest((unsigned char*)test[i][0].c_str(),
        test[i][0].length(), digest);
    EXPECT_EQ(toHex(digest, digest_size), test[i][1]);
  }

  delete[] digest;
}

TEST(keccak_256_cryptopp, update_and_finish) {
  Keccak_256_cryptopp hasher;
  size_t digest_size = hasher.digest_size();
  unsigned char* digest = new unsigned char[digest_size];
  std::string test_input(
      "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno");
  const std::string EXP1 =
      "C8A625720D2C6221C09DB8A33A63FB936E628A0C10195768A206E7AD8D1E54DE";
  unsigned char* p_test_input = (unsigned char*) test_input.c_str();
  size_t len = test_input.length();

  for (unsigned int i = 0; i < 10; i++) {
    hasher.update(p_test_input, len);
  }
  hasher.final(digest);

  EXPECT_EQ(toHex(digest, digest_size), EXP1);

  // Try to hash a new string to see if everything restarted as intended
  unsigned char* a = (unsigned char*) "a";
  unsigned char* b = (unsigned char*) "b";
  unsigned char* c = (unsigned char*) "c";
  const std::string EXP2 =
      "4E03657AEA45A94FC7D47BA826C8D667C0D1E6E33A64A036EC44F58FA12D6C45";
  hasher.update(a, 1);
  hasher.update(b, 1);
  hasher.update(c, 1);
  hasher.final(digest);

  EXPECT_EQ(toHex(digest, digest_size), EXP2);

  delete[] digest;
}

TEST(keccak_256_cryptopp, digest_size) {
  Keccak_256_cryptopp hasher;
  EXPECT_EQ(hasher.digest_size(), CryptoPP::Keccak_256::DIGESTSIZE);
}
