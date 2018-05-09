#include <fstream>
#include <iostream>
#include <string>

#include "data/protobuf/protobuf_factory.h"
#include "data/protobuf/protobuf_schema.h"
#include "gtest/gtest.h"

using data::msg;
using data::schema;
using data::protobuf::protobuf_factory;
using data::protobuf::protobuf_schema;

TEST(protobuf_factory, enums) {
  /**

    message A {
      string string_field = 1;
      inner_enum inner_enum_field = 2;
      outer_enum outer_enum_field = 3;

      enum inner_enum {
        inner_value1 = 0;
        inner_value2 = 1;
      }
    }

    enum outer_enum {
      outer_value1 = 0;
      outer_value2 = 1;
    }

    message B {
      A.inner_enum enum_field1 = 1;
      repeated outer_enum enum_field2 = 2;
    }

  **/

  protobuf_schema custom_schema;
  int m1 = custom_schema.create_message("A");
  int m2 = custom_schema.create_message("B");
  int e1 = custom_schema.create_enum("inner_enum");
  int e2 = custom_schema.create_enum("outer_enum");

  custom_schema.add_scalar_field(schema::field_info(1,
      schema::string, "string_field", "", false), m1);
  custom_schema.add_enum_field(schema::field_info(2,
      schema::enum_type, "inner_enum_field",
          "A.inner_enum", false), m1);
  custom_schema.add_enum_field(schema::field_info(3,
      schema::enum_type, "outer_enum_field",
      "outer_enum", false), m1);
  custom_schema.add_enum_field(schema::field_info(1,
        schema::enum_type, "enum_field1", "A.inner_enum",
        false), m2);
  custom_schema.add_enum_field(schema::field_info(2,
      schema::enum_type, "enum_field2", "outer_enum",
      true), m2);

  custom_schema.add_enum_value(e1, "inner_value1", 0);
  custom_schema.add_enum_value(e1, "inner_value2", 1);
  custom_schema.add_enum_value(e2, "outer_value1", 0);
  custom_schema.add_enum_value(e2, "outer_value2", 1);

  custom_schema.add_enum(e1, m1);
  custom_schema.add_enum(e2, -1);
  custom_schema.add_message(m1);
  custom_schema.add_message(m2);

  protobuf_factory sc;
  sc.import_schema_definition(&custom_schema, "test", "");

  msg * msg1 = sc.new_message("A");
  msg * msg2 = sc.new_message("A");

  msg1->set_string(1, "value_string");
  int inner_enum_value = sc.get_enum_value(sc.get_enum_id("A.inner_enum"),
      "inner_value2");
  msg1->set_enum(2, inner_enum_value);
  int outer_enum_value = sc.get_enum_value(sc.get_enum_id("outer_enum"),
      "outer_value1");
  msg1->set_enum(3, outer_enum_value);

  std::cout << "Message {\n" << msg1->to_string() << "\n}" << std::endl;

  std::string data1;
  msg1->serialize_message(&data1);
  msg2->deserialize_message(data1);

  EXPECT_EQ(msg2->get_string(1), "value_string");
  EXPECT_EQ(msg2->get_enum(2), 1);
  EXPECT_EQ(msg2->get_enum(3), 0);

  msg * msg3 = sc.new_message(sc.get_schema_id("B"));
  msg * msg4 = sc.new_message(sc.get_schema_id("B"));

  msg3->set_enum(1, 1);
  msg3->set_repeated_enum(2, 1, -1);
  msg3->set_repeated_enum(2, 0, -1);
  msg3->set_repeated_enum(2, 1, -1);

  EXPECT_EQ(msg3->get_enum(1), 1);
  EXPECT_EQ(msg3->get_repeated_field_size(2), 3);
  EXPECT_EQ(msg3->get_repeated_enum(2, 0), 1);
  EXPECT_EQ(msg3->get_repeated_enum(2, 1), 0);
  EXPECT_EQ(msg3->get_repeated_enum(2, 2), 1);

  std::string data2;
  msg3->serialize_message(&data2);
  msg4->deserialize_message(data2);

  EXPECT_EQ(msg4->get_enum(1), 1);
  EXPECT_EQ(msg4->get_repeated_field_size(2), 3);
  EXPECT_EQ(msg4->get_repeated_enum(2, 0), 1);
  EXPECT_EQ(msg4->get_repeated_enum(2, 1), 0);
  EXPECT_EQ(msg4->get_repeated_enum(2, 2), 1);

  msg3->set_repeated_enum(2, 1, 1);
  EXPECT_EQ(msg3->get_repeated_enum(2, 1), 1);
  try {
    msg3->set_enum(2, 1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    EXPECT_EQ(message, "Field is repeated");
  }
  google::protobuf::ShutdownProtobufLibrary();
}
