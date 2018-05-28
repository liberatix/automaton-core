#include "automaton/core/storage/blobstore.h"
#include "gtest/gtest.h"
#include <cstring>
#include <random>
#include <vector>
#include <iostream>

using automaton::core::storage::blobstore;

TEST(blobstore, basic_test) {
  blobstore storage("test");
  uint32_t blob_size = 16;
  uint64_t access_id;
  std::string data_to_save = "0123456789ABCDEF";
  uint8_t* p_blob = storage.create_blob(blob_size, &access_id);
  std::memcpy(p_blob, &data_to_save[0], blob_size);
  EXPECT_FALSE(std::memcmp(p_blob, &data_to_save[0], blob_size));

  uint32_t retrived_size = 0;
  uint8_t* retrived_blob = storage.get_data(access_id, &retrived_size);
  EXPECT_EQ(retrived_size, blob_size);
  EXPECT_FALSE(std::memcmp(retrived_blob, &data_to_save[0], blob_size));
}


TEST(blobstore, create_blob_write_get_data) {
  blobstore storage("test");
  std::vector<std::string> tests;
  std::vector<uint64_t> access_id;
  std::random_device rnd;

  tests.push_back("Automaton");
  tests.push_back("Liberatix");
  tests.push_back("Rick and morty");
  tests.push_back("Betahaus");
  tests.push_back("You should not waste your time reading the actuall strings, those are just"
      "test inputs");
  tests.push_back("The end is coming!!!");
  tests.push_back("Did I miss it?");
  tests.push_back("He's almost been right so many times. He was sure it was coming during "
      "the Cataclysm.");
  tests.push_back("You must be boored, fine here are some good references:");
  tests.push_back("If you can’t tell the difference, does it matter if I'm real or not");
  tests.push_back("Someday sounds a lot like the thing people say when they actually mean never.");
  tests.push_back("In 900 years of time and space, I’ve never met anyone who wasn’t important");
  tests.push_back("Nobody exists on purpose. Nobody belongs anywhere. We're all going to die. "
      "Come watch TV.");
  tests.push_back("The answer is: Don't think about it.");

  for (const auto &test : tests) {
    size_t blob_size = test.length();
    access_id.push_back(0ULL);
    uint8_t* p_blob = storage.create_blob(blob_size, &access_id.back());
    std::memcpy(p_blob, &test[0] , blob_size);
    EXPECT_FALSE(std::memcmp(p_blob, &test[0], blob_size));

    uint32_t retrived_size = 0;
    uint8_t* retrived_blob = storage.get_data(access_id.back(), &retrived_size);
    EXPECT_EQ(retrived_size, blob_size);
    EXPECT_FALSE(std::memcmp(retrived_blob,  &test[0], blob_size));
  }
}
