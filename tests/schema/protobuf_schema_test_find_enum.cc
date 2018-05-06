#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"
#include "schema/protobuf_schema_definition.h"
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

TEST(protobuf_schema, find_enum) {
  protobuf_schema sc;
  sc.import_schema_from_string(file_to_bytes("tests/schema/many_enums.proto"),
  "test", "");
  int id = sc.get_enum_id("enum5");
  std::vector <std::pair<std::string, int> > info = sc.get_enum_values(id);
  EXPECT_EQ(info[0].first, "a");
  EXPECT_EQ(info[0].second, 0);
  EXPECT_EQ(info[1].first, "b");
  EXPECT_EQ(info[1].second, 1);
  google::protobuf::ShutdownProtobufLibrary();
}
