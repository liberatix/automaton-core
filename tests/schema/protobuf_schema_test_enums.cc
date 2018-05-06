#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"
#include "schema/protobuf_schema_definition.h"
#include "gtest/gtest.h"

TEST(protobuf_schema, enums) {
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

  protobuf_schema_definition custom_schema;
  int m1 = custom_schema.create_message("A");
  int m2 = custom_schema.create_message("B");
  int e1 = custom_schema.create_enum("inner_enum");
  int e2 = custom_schema.create_enum("outer_enum");

  custom_schema.add_scalar_field(schema_definition::field_info(1,
      schema_definition::field_type::string, "string_field", "", false), m1);
  custom_schema.add_enum_field(schema_definition::field_info(2,
      schema_definition::field_type::enum_type, "inner_enum_field",
          "A.inner_enum", false), m1);
  custom_schema.add_enum_field(schema_definition::field_info(3,
      schema_definition::field_type::enum_type, "outer_enum_field",
      "outer_enum", false), m1);
  custom_schema.add_enum_field(schema_definition::field_info(1,
        schema_definition::field_type::enum_type, "enum_field1", "A.inner_enum",
        false), m2);
  custom_schema.add_enum_field(schema_definition::field_info(2,
      schema_definition::field_type::enum_type, "enum_field2", "outer_enum",
      true), m2);

  custom_schema.add_enum_value(e1, "inner_value1", 0);
  custom_schema.add_enum_value(e1, "inner_value2", 1);
  custom_schema.add_enum_value(e2, "outer_value1", 0);
  custom_schema.add_enum_value(e2, "outer_value2", 1);

  custom_schema.add_enum(e1, m1);
  custom_schema.add_enum(e2, -1);
  custom_schema.add_message(m1);
  custom_schema.add_message(m2);

  protobuf_schema sc;
  sc.import_schema_definition(&custom_schema, "test", "");

  int id1 = sc.new_message(sc.get_schema_id("A"));
  int id2 = sc.new_message(sc.get_schema_id("A"));

  sc.set_string(id1, 1, "value_string");
  int inner_enum_value = sc.get_enum_value(sc.get_enum_id("A.inner_enum"),
      "inner_value2");
  sc.set_enum(id1, 2, inner_enum_value);
  int outer_enum_value = sc.get_enum_value(sc.get_enum_id("outer_enum"),
      "outer_value1");
  sc.set_enum(id1, 3, outer_enum_value);

  std::cout << "Message {\n" << sc.to_string(id1) << "\n}" << std::endl;

  std::string data1;
  sc.serialize_message(id1, &data1);
  sc.deserialize_message(id2, data1);

  EXPECT_EQ(sc.get_string(id2, 1), "value_string");
  EXPECT_EQ(sc.get_enum(id2, 2), 1);
  EXPECT_EQ(sc.get_enum(id2, 3), 0);

  int id3 = sc.new_message(sc.get_schema_id("B"));
  int id4 = sc.new_message(sc.get_schema_id("B"));

  sc.set_enum(id3, 1, 1);
  sc.set_repeated_enum(id3, 2, 1, -1);
  sc.set_repeated_enum(id3, 2, 0, -1);
  sc.set_repeated_enum(id3, 2, 1, -1);

  EXPECT_EQ(sc.get_enum(id3, 1), 1);
  EXPECT_EQ(sc.get_repeated_field_size(id3, 2), 3);
  EXPECT_EQ(sc.get_repeated_enum(id3, 2, 0), 1);
  EXPECT_EQ(sc.get_repeated_enum(id3, 2, 1), 0);
  EXPECT_EQ(sc.get_repeated_enum(id3, 2, 2), 1);

  std::string data2;
  sc.serialize_message(id3, &data2);
  sc.deserialize_message(id4, data2);

  EXPECT_EQ(sc.get_enum(id4, 1), 1);
  EXPECT_EQ(sc.get_repeated_field_size(id4, 2), 3);
  EXPECT_EQ(sc.get_repeated_enum(id4, 2, 0), 1);
  EXPECT_EQ(sc.get_repeated_enum(id4, 2, 1), 0);
  EXPECT_EQ(sc.get_repeated_enum(id4, 2, 2), 1);

  sc.set_repeated_enum(id3, 2, 1, 1);
  EXPECT_EQ(sc.get_repeated_enum(id3, 2, 1), 1);
  try {
    sc.set_enum(id3, 2, 1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    EXPECT_EQ(message, "Field is repeated");
  }
  google::protobuf::ShutdownProtobufLibrary();
}
