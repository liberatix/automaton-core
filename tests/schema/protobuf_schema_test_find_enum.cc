#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "schema/protobuf_schema.h"
#include "schema/protobuf_schema_definition.h"
#include "gtest/gtest.h"

TEST(protobuf_schema, find_enum) {
  protobuf_schema sc;
  sc.import_schema_from_string(
      get_file_contents("tests/schema/many_enums.proto"), "test", "");
  int id = sc.get_enum_id("enum5");
  std::vector <std::pair<std::string, int> > info = sc.get_enum_values(id);
  EXPECT_EQ(info[0].first, "a");
  EXPECT_EQ(info[0].second, 0);
  EXPECT_EQ(info[1].first, "b");
  EXPECT_EQ(info[1].second, 1);
  google::protobuf::ShutdownProtobufLibrary();
}
