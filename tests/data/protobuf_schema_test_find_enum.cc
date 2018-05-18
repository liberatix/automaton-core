#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "data/protobuf/protobuf_factory.h"
#include "data/protobuf/protobuf_schema.h"
#include "gtest/gtest.h"

using data::protobuf::protobuf_factory;

TEST(protobuf_factory, find_enum) {
  protobuf_factory sc;
  sc.import_schema_from_string(
      get_file_contents("tests/data/many_enums.proto"), "test", "");
  int id = sc.get_enum_id("enum5");
  std::vector <std::pair<std::string, int> > info = sc.get_enum_values(id);
  EXPECT_EQ(info[0].first, "a");
  EXPECT_EQ(info[0].second, 0);
  EXPECT_EQ(info[1].first, "b");
  EXPECT_EQ(info[1].second, 1);
  google::protobuf::ShutdownProtobufLibrary();
}
