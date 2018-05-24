#include "automaton/core/crypto/hash_transformation.h"
#include "gtest/gtest.h"

using automaton::core::crypto::hash_transformation;

template<uint8_t C>
class dummy_hash : public hash_transformation {
 public:
  void update(const uint8_t * input, const size_t length) {}

  void final(uint8_t * digest) {
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

const uint8_t * TEST1 = (const uint8_t*)"abc";
const uint8_t * TEST2 = (const uint8_t*)"test";

// Tests hash transformation registration.
TEST(HashTranformation, Registration) {
  hash_transformation::register_factory(DUMMY1,
      [] {
        return reinterpret_cast<hash_transformation*>(new dummy_hash<1>());
      });
  hash_transformation::register_factory(DUMMY2,
      [] {
        return reinterpret_cast<hash_transformation*>(new dummy_hash<2>());
      });

  uint8_t digest[1];

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
