#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "data/protobuf/protobuf_factory.h"
#include "gtest/gtest.h"

using data::protobuf::protobuf_factory;

TEST(protobuf_factory, find_all_fields) {
  protobuf_factory sc;
  sc.import_schema_from_string(
      get_file_contents("tests/data/many_fields.proto"), "test", "");
  int k;
  int id = sc.get_schema_id("TestMsg");
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
  google::protobuf::ShutdownProtobufLibrary();
}
