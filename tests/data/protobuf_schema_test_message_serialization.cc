#include <fstream>
#include <iostream>
#include <string>

#include "data/protobuf/protobuf_factory.h"
#include "data/protobuf/protobuf_schema.h"
#include "gtest/gtest.h"

using data::msg;
using data::protobuf::protobuf_factory;
using data::protobuf::protobuf_schema;

TEST(protobuf_factory, message_serialization) {
  /**
    first_message {
      string string_field = 1;
      repeated int32 nint32_field = 2;
    }
  **/
  protobuf_schema custom_schema;
  int m1 = custom_schema.create_message("first_message");
  custom_schema.add_scalar_field(schema::field_info(1,
      schema::string, "string_field", "", false), m1);
  custom_schema.add_scalar_field(schema::field_info(2,
      schema::int32, "int32_field", "", true), m1);
  custom_schema.add_message(m1);

  protobuf_factory sc;
  sc.import_schema_definition(&custom_schema, "test", "");

  auto msg1 = sc.new_message(0);
  auto msg2 = sc.new_message(0);

  msg1->set_string(1, "value");
  msg1->set_repeated_int32(2, 7, -1);
  msg1->set_repeated_int32(2, 11, -1);

  std::string data;
  msg1->serialize_message(&data);
  msg2->deserialize_message(data);

  EXPECT_EQ(msg1->get_string(1), "value");
  EXPECT_EQ(msg2->get_string(1), msg1->get_string(1));
  EXPECT_EQ(msg2->get_repeated_field_size(2), 2);
  EXPECT_EQ(msg2->get_repeated_int32(2, 0), 7);
  EXPECT_EQ(msg2->get_repeated_int32(2, 1), 11);

  google::protobuf::ShutdownProtobufLibrary();
}
