#include "automaton/core/storage/persistent_blobstore.h"
#include <cstring>
#include <random>
#include <vector>
#include <iostream>
#include "automaton/core/log/log.h"
#include "gtest/gtest.h"

using automaton::core::storage::persistent_blobstore;

TEST(persistent_blobstore, create_mapped_file) {

  persistent_blobstore bs1;
  bs1.map_file("mapped_file.txt");

  std::string data1 = "data1";
  std::string data20 = "data20";
  std::string much_longer_string = "much_longer_string";
  uint64_t ids[10];
  LOG(INFO) << "Reached point 1";
  ids[0] = bs1.store(data1.size(), (uint8_t*) data1.c_str());
  LOG(INFO) << "Reached point 2";
  ids[1] = bs1.store(data20.size(), (uint8_t*) data20.c_str());
  ids[2] = bs1.store(data20.size(), (uint8_t*) data20.c_str());
  ids[3] = bs1.store(much_longer_string.size(), (uint8_t*) much_longer_string.c_str());
  ids[4] = bs1.store(data1.size(), (uint8_t*) data1.c_str());
  ids[5] = bs1.store(data20.size(), (uint8_t*) data20.c_str());
  LOG(INFO) << "Reached point 3";
  uint32_t sz;
  //string data;
  uint8_t* pData;
  for (int i = 0; i < 6; i++) {
    pData = bs1.get(ids[i], &sz);
    std::cout << std::string((char*)pData, sz) << std::endl;
  }
  LOG(INFO) << "Reached point 4";
}
