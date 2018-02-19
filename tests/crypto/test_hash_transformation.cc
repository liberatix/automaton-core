#include "crypto/hash_transformation.h"
#include "gtest/gtest.h"

class dummy_hash : public hash_transformation {
private:
  char hash;
public:
  dummy_hash():hash(0) {}

  void update(const unsigned char * input, const size_t length) {
    for (unsigned int i = 0; i < length; i++) {
      hash ^= input[i];
    }
  }

  void final(unsigned char * digest) {
    *digest = hash;
  }

  void restart() {
    hash = 0;
  }

  unsigned int digest_size() const {
    return 1;
  }
};

const char* DUMMY = "dummy";
const char* DUMMY_BAD = "dummy/bad";

// Tests hash transformation registration.
TEST(HashTranformation, Registration) {
  hash_transformation::register_class_factory(DUMMY,
            [] {return (hash_transformation*)new dummy_hash(); });
  hash_transformation * d = hash_transformation::create(DUMMY);
  EXPECT_NE(d, nullptr);
}

// Tests hash transformation creation.
TEST(HashTranformation, CreateBadHashTransformation) {
  hash_transformation * d = hash_transformation::create(DUMMY_BAD);
  EXPECT_EQ(d, nullptr);
}
