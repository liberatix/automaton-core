#include <fstream>
#include <iostream>
#include <string>

#include "automaton/core/io/io.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"

#include "gtest/gtest.h"

using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::io::get_file_contents;

TEST(protobuf_factory, find_all_enums) {
  protobuf_factory sc;
  sc.import_schema_from_string(
      get_file_contents("automaton/tests/data/many_enums.proto"), "test", "");
  int k = sc.get_enums_number();
  EXPECT_EQ(k, 8);
  google::protobuf::ShutdownProtobufLibrary();
}
