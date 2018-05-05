#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"
#include "gtest/gtest.h"

TEST(protobuf_schema, message_serialization) {
  /**
    first_message {
      string string_field = 1;
      repeated int32 nint32_field = 2;
    }
  **/
  protobuf_schema_definition custom_schema;
  int m1 = custom_schema.create_message("first_message");
  custom_schema.add_scalar_field(schema_definition::field_info(1,
      schema_definition::field_type::string, "string_field", "", false), m1);
  custom_schema.add_scalar_field(schema_definition::field_info(2,
      schema_definition::field_type::int32, "int32_field", "", true), m1);
  custom_schema.add_message(m1);

  protobuf_schema sc;
  sc.import_schema_definition(&custom_schema, "test", "");

  int id1 = sc.new_message(0);
  int id2 = sc.new_message(0);

  sc.set_string(id1, 1, "value");
  sc.set_repeated_int32(id1, 2, 7, -1);
  sc.set_repeated_int32(id1, 2, 11, -1);

  std::string message;
  sc.serialize_message(id1, &message);
  sc.deserialize_message(id2, message);
  EXPECT_EQ(sc.get_string(id1, 1), "value");
  EXPECT_EQ(sc.get_string(id2, 1), sc.get_string(id1, 1));
  EXPECT_EQ(sc.get_repeated_field_size(id2, 2), 2);
  EXPECT_EQ(sc.get_repeated_int32(id2, 2, 0), 7);
  EXPECT_EQ(sc.get_repeated_int32(id2, 2, 1), 11);
  google::protobuf::ShutdownProtobufLibrary();
}
