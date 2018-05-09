#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "data/protobuf_schema.h"

#include "gtest/gtest.h"

TEST(protobuf_schema, find_all_enums) {
  protobuf_schema sc;
  sc.import_schema_from_string(
      get_file_contents("tests/data/many_enums.proto"), "test", "");
  int k = sc.get_enums_number();
  EXPECT_EQ(k, 8);
  google::protobuf::ShutdownProtobufLibrary();
}
