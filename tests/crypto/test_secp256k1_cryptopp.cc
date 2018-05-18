#include <string>
#include <vector>
#include "crypto/digital_signature.h"
#include "crypto/cryptopp/secp256k1_cryptopp.h"
#include "gtest/gtest.h"
#include "hex.h"  // NOLINT
#include "filters.h"  // NOLINT

// Helper function to convert bytes to hex values
// Each byte is converted to 2 hex values, encoding the left and
// right 4 bits of each byte.
static std::string toHex(unsigned char * decoded, size_t size) {
  std::string output;
  CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(output), false);
  encoder.Put(decoded, size);
  encoder.MessageEnd();
  return output;
}
void decode_from_hex(std::string &encoded, std::string &decoded) {   // NOLINT
  CryptoPP::StringSource ss(encoded, true,
    new CryptoPP::HexDecoder(new CryptoPP::StringSink(decoded)));
}
TEST(secp256k1_cryptopp, gen_public_key) {
  secp256k1_cryptopp::register_self();
  digital_signature* tester = digital_signature::create("secp256k1");
  EXPECT_NE(tester, nullptr);
  unsigned char* public_key = new unsigned char[tester->public_key_size()];
  constexpr unsigned int test_cases = 4;
  std::string test[test_cases][2] = {
    {"5f3aa3bb3129db966915a6d341fde4c95121b5f4cedc3ba4ecc3dd44ba9a50bc",
     "02b4a66219f8e6e594979d8c1961be1aa98e8384b534d54519217e0fbbe4ea608d"},
    {"77f8406c4620450c9bb233e6cc404bb23a6bf86af3c943df8f0710f612d7ff23",
     "0201bddf939a6ab9acf928a38c5973c752d8018d17e9f24d09287d2c0a2c06f852"},
    {"b33230bf39182dc6e158d686c8b614fa24d80ac6db8cfa13465faedb12edf6a4",
     "03a09c9e2f6472a1e73ac5a4a9a97b09d9391f57d3a38d1733b7cf672cfc645699"},
    {"e3b0f44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
     "0308e9692dd9f3cd5061167d7bff031c76c63f5bd492909f26826fe399bda2e1eb"}
  };
  for (unsigned int i = 0; i < test_cases; i++) {
    std::string pr_key_decoded;
    decode_from_hex(test[i][0], pr_key_decoded);
    tester->gen_public_key((unsigned char*)pr_key_decoded.c_str(), public_key);
    EXPECT_EQ(test[i][1], toHex(public_key, tester->public_key_size()));
  }
}
TEST(secp256k1_cryptopp, sign_and_verify) {
  secp256k1_cryptopp::register_self();
  digital_signature* tester = digital_signature::create("secp256k1");
  EXPECT_NE(tester, nullptr);
  unsigned char* public_key = new unsigned char[tester->public_key_size()];
  unsigned char* signature = new unsigned char[tester->signature_size()];
  std::vector<std::string> test_key = {
    "5f3aa3bb3129db966915a6d341fde4c95121b5f4cedc3ba4ecc3dd44ba9a50bc",
    "77f8406c4620450c9bb233e6cc404bb23a6bf86af3c943df8f0710f612d7ff23",
    "b33230bf39182dc6e158d686c8b614fa24d80ac6db8cfa13465faedb12edf6a4",
    "e3b0f44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
  };
  std::vector<std::string> test_hash = {
    "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD",
    "HELLO, HELLO, HELLO",
    "We could have been friends",
    "Final Space",
  };
  for (unsigned int i = 0; i < test_key.size(); i++) {
    std::string pr_key_decoded;
    decode_from_hex(test_key[i], pr_key_decoded);
    tester->gen_public_key((unsigned char*)pr_key_decoded.c_str(), public_key);
    for (unsigned int j = 0; j < test_hash.size(); j++) {
      tester->sign((unsigned char*)pr_key_decoded.c_str(),
                  (unsigned char*) test_hash[j].c_str(),
                  test_hash[j].length(),
                  signature);
      EXPECT_EQ(tester->verify(public_key,
                              (unsigned char*) test_hash[j].c_str(),
                              test_hash[j].length(),
                              signature), true);
    }
  }
}
TEST(secp256k1_cryptopp, verify) {
}
TEST(secp256k1_cryptopp, check_return_sizes) {
}
