#include <string>
#include "automaton/core/crypto/hash_transformation.h"
#include "automaton/core/crypto/cryptopp/SHA256_cryptopp.h"
#include "cryptlib.h"  // NOLINT
#include "sha.h"  // NOLINT
#include "gtest/gtest.h"  // NOLINT
#include "hex.h"  // NOLINT
#include "filters.h"  // NOLINT

using automaton::core::crypto::SHA256_cryptopp;
using automaton::core::crypto::hash_transformation;

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


TEST(SHA256_cryptopp, calculate_digest) {
  SHA256_cryptopp hasher;
  size_t digest_size = hasher.digest_size();
  unsigned char* digest = new unsigned char[digest_size];
  constexpr unsigned int test_cases = 6;
  std::string long_a_string(1000000, 'a');

  std::string test[test_cases][2] = {
    {"a", "CA978112CA1BBDCAFAC231B39A23DC4DA786EFF8147C4E72B9807785AFEE48BB"},
    // NOLINTNEXTLINE
    {"abc", "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD"},
    {"", "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855"},
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
     "248D6A61D20638B8E5C026930C3E6039A33CE45964FF2167F6ECEDD419DB06C1"},
    // NOLINTNEXTLINE
    {"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
     "CF5B16A778AF8380036CE59E7B0492370B249B11E8F07A51AFAC45037AFEE9D1"},
    {long_a_string,
      "CDC76E5C9914FB9281A1C7E284D73E67F1809A48A497200E046D39CCC7112CD0"}
  };

  for (unsigned int i = 0; i < test_cases; i++) {
    hasher.calculate_digest((unsigned char*)test[i][0].c_str(),
        test[i][0].length(), digest);
    EXPECT_EQ(toHex(digest, digest_size), test[i][1]);
  }

  delete[] digest;
}

TEST(SHA256_cryptopp, update_and_finish) {
  SHA256_cryptopp hasher;
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
  EXPECT_EQ(toHex(digest, digest_size),
      "50E72A0E26442FE2552DC3938AC58658228C0CBFB1D2CA872AE435266FCD055E");

  // Try to hash a new string to see if everything restarted as intended
  unsigned char* a = (unsigned char*) "a";
  unsigned char* b = (unsigned char*) "b";
  unsigned char* c = (unsigned char*) "c";
  hasher.update(a, 1);
  hasher.update(b, 1);
  hasher.update(c, 1);
  hasher.final(digest);
  EXPECT_EQ(toHex(digest, digest_size),
      "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD");

  delete[] digest;
}

TEST(SHA256_cryptopp, digest_size) {
  SHA256_cryptopp hasher;
  EXPECT_EQ(hasher.digest_size(), CryptoPP::SHA256::DIGESTSIZE);
}
