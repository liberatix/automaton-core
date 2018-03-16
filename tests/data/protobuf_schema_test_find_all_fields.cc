#include <fstream>
#include <iostream>
#include <string>

#include "data/protobuf_schema.h"
#include "gtest/gtest.h"

std::string file_to_bytes(char const* filename) {
  std::ifstream is(filename, std::ifstream::binary);
  if (is) {
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    char* buffer = new char[length];
    std::cout << "Reading " << length << " characters... ";
    is.read(buffer, length);
    if (is) {
      std::cout << "all characters read successfully." << std::endl;
    } else {
      std::cout << "error: only " << is.gcount() << " could be read"
          << std::endl;
    }
    is.close();
    return std::string(buffer, length);
  }
  return "";
}

TEST(protobuf_schema, finding_all_fields) {
  protobuf_schema sc;
  sc.import_schema_from_string(file_to_bytes("tests/data/many_fields.proto"),
  "test", "");
  int k;
  int id  = sc.get_schema_id("TestMsg");
  k = sc.get_fields_number(id);
  EXPECT_EQ(k, 0);
  id = sc.get_schema_id("TestMsg2");
  k = sc.get_fields_number(id);
  EXPECT_EQ(k, 7);
  id  = sc.get_schema_id("TestMsg4.TestMsg5");
  k = sc.get_fields_number(id);
  EXPECT_EQ(k, 1);
  id  = sc.get_schema_id("TestMsg4.TestMsg5.TestMsg6");
  k = sc.get_fields_number(id);
  EXPECT_EQ(k, 1);
}
