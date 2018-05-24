#include <string>
#include <vector>
#include "automaton/core/crypto/digital_signature.h"
#include "automaton/core/crypto/ed25519_orlp/ed25519_orlp.h"
#include "gtest/gtest.h"
#include "hex.h"  // NOLINT
#include "filters.h"  // NOLINT

using automaton::core::crypto::digital_signature;
using automaton::core::crypto::ed25519_orlp;

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
}
TEST(secp256k1_cryptopp, sign_and_verify) {
  ed25519_orlp::register_self();

  digital_signature * tester = digital_signature::create("ed25519_orlp");
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
