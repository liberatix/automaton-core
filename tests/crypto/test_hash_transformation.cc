#include "crypto/hash_transformation.h"
#include "gtest/gtest.h"

template<unsigned char C>
class dummy_hash : public hash_transformation {
public:
  void update(const unsigned char * input, const size_t length) {}

  void final(unsigned char * digest) {
    digest[0] = C;
  }

  void restart() {}

  unsigned int digest_size() const {
    return 1;
  }
};

const char* DUMMY1 = "dummy1";
const char* DUMMY2 = "dummy2";
const char* DUMMY3 = "dummy3";

const unsigned char * TEST1 = (const unsigned char*)"abc";
const unsigned char * TEST2 = (const unsigned char*)"test";

// Tests hash transformation registration.
TEST(HashTranformation, Registration) {
  hash_transformation::register_class_factory(DUMMY1,
            [] {return (hash_transformation*)new dummy_hash<1>(); });
  hash_transformation::register_class_factory(DUMMY2,
            [] {return (hash_transformation*)new dummy_hash<2>(); });

  unsigned char digest[1];

  hash_transformation * d1 = hash_transformation::create(DUMMY1);
  EXPECT_NE(d1, nullptr);
  d1->calculate_digest(TEST1, strlen((const char*)TEST1), digest);
  EXPECT_EQ(digest[0], 1);

  hash_transformation * d2 = hash_transformation::create(DUMMY2);
  EXPECT_NE(d2, nullptr);
  d2->calculate_digest(TEST1, strlen((const char*)TEST1), digest);
  EXPECT_EQ(digest[0], 2);
}

// Tests hash transformation creation.
TEST(HashTranformation, CreateBadHashTransformation) {
  hash_transformation * d = hash_transformation::create(DUMMY3);
  EXPECT_EQ(d, nullptr);
}
