#include "crypto/hash_transformation.h"
#include "crypto/SHA256_cryptopp.h"
#include "gtest/gtest.h"


const char* SHA256_REGISTRATION = "SHA256";

TEST(SHA256_cryptoPP, Create) {
  //hash_transformation * pSHA256 = new SHA256_cryptoPP();
  SHA256_cryptoPP _sha256;
  int v = 1;
  v /= 1;
  EXPECT_EQ(v,1);
}
