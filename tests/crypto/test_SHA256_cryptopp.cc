#include "crypto/hash_transformation.h"
#include "crypto/SHA256_cryptopp.h"
#include "cryptlib.h"
#include "sha.h"
#include "gtest/gtest.h"

const char* SHA256_REGISTRATION = "SHA256";

TEST(SHA256_cryptopp, Create) {
  //hash_transformation * pSHA256 = new SHA256_cryptoPP();
  //SHA256_cryptoPP _sha256;
  CryptoPP::SHA256 _sha_test_obj_;


}
