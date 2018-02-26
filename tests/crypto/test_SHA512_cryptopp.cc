#include <string>
#include "crypto/hash_transformation.h"
#include "crypto/SHA512_cryptopp.h"
#include "cryptlib.h"  // NOLINT
#include "sha.h"  // NOLINT
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

TEST(SHA512_cryptopp, register_self) {
  SHA512_cryptopp::register_self();
  hash_transformation * hasher = hash_transformation::create("SHA512");
  EXPECT_NE(hasher, nullptr);
}

TEST(SHA512_cryptopp, calculate_digest) {
  SHA512_cryptopp hasher;
  size_t digest_size = hasher.digest_size();
  unsigned char* digest = new unsigned char[digest_size];
  constexpr unsigned int test_cases = 6;
  std::string long_a_string(1000000, 'a');

  std::string test[test_cases][2] = {
    {"a",
    // NOLINTNEXTLINE
     "1F40FC92DA241694750979EE6CF582F2D5D7D28E18335DE05ABC54D0560E0F5302860C652BF08D560252AA5E74210546F369FBBBCE8C12CFC7957B2652FE9A75"},
    {"abc",
    // NOLINTNEXTLINE
     "DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A2192992A274FC1A836BA3C23A3FEEBBD454D4423643CE80E2A9AC94FA54CA49F"},
    {"",
    // NOLINTNEXTLINE
     "CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE47D0D13C5D85F2B0FF8318D2877EEC2F63B931BD47417A81A538327AF927DA3E"},
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    // NOLINTNEXTLINE
     "204A8FC6DDA82F0A0CED7BEB8E08A41657C16EF468B228A8279BE331A703C33596FD15C13B1B07F9AA1D3BEA57789CA031AD85C7A71DD70354EC631238CA3445"},
    // NOLINTNEXTLINE
    {"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
    // NOLINTNEXTLINE
     "8E959B75DAE313DA8CF4F72814FC143F8F7779C6EB9F7FA17299AEADB6889018501D289E4900F7E4331B99DEC4B5433AC7D329EEB6DD26545E96E55B874BE909"},
    {long_a_string,
      // NOLINTNEXTLINE
      "E718483D0CE769644E2E42C7BC15B4638E1F98B13B2044285632A803AFA973EBDE0FF244877EA60A4CB0432CE577C31BEB009C5C2C49AA2E4EADB217AD8CC09B"}
  };

  for (unsigned int i = 0; i < test_cases; i++) {
    hasher.calculate_digest((unsigned char*)test[i][0].c_str(),
        test[i][0].length(), digest);
    EXPECT_EQ(toHex(digest, digest_size), test[i][1]);
  }
}

TEST(SHA512_cryptopp, update_and_finish) {
  SHA512_cryptopp hasher;
  size_t digest_size = hasher.digest_size();
  unsigned char* digest = new unsigned char[digest_size];
  std::string test_input(
      "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno");
  unsigned char* p_test_input = (unsigned char*) test_input.c_str();
  size_t len = test_input.length();

  for (unsigned int i = 0; i <  16777216; i++) {
    hasher.update(p_test_input, len);
  }
  hasher.final(digest);
  // NOLINTNEXTLINE
  EXPECT_EQ(toHex(digest, digest_size),
      "B47C933421EA2DB149AD6E10FCE6C7F93D0752380180FFD7F4629A712134831D77BE6091B819ED352C2967A2E2D4FA5050723C9630691F1A05A7281DBE6C1086");

  // Try to hash a new string to see if everything restarted as intended
  unsigned char* a = (unsigned char*) "a";
  unsigned char* b = (unsigned char*) "b";
  unsigned char* c = (unsigned char*) "c";
  hasher.update(a, 1);
  hasher.update(b, 1);
  hasher.update(c, 1);
  hasher.final(digest);
  // NOLINTNEXTLINE
  EXPECT_EQ(toHex(digest, digest_size),
      "DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A2192992A274FC1A836BA3C23A3FEEBBD454D4423643CE80E2A9AC94FA54CA49F");
}

TEST(SHA512_cryptopp, digest_size) {
  SHA512_cryptopp hasher;
  EXPECT_EQ(hasher.digest_size(), CryptoPP::SHA512::DIGESTSIZE);
}
