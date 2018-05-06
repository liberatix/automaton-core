#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"
#include "schema/protobuf_schema_definition.h"
#include "gtest/gtest.h"

TEST(protobuf_schema, messages) {
  /**

    first_message {
      string string_field_1 = 1;
      second_message message_field = 2;

      nested_message {
        string string_field_nested = 1;
      }
    }
    second_message {
      string string_field_2 = 1;
    }
    third_message {
      repeated second_message repeated_msg_field = 1;
    }

  **/

  protobuf_schema_definition custom_schema;
  int m1 = custom_schema.create_message("first_message");
  int m2 = custom_schema.create_message("second_message");
  int m3 = custom_schema.create_message("nested_message");
  int m4 = custom_schema.create_message("third_message");

  custom_schema.add_scalar_field(schema_definition::field_info(1,
      schema_definition::field_type::string, "string_field_1", "", false), m1);
  custom_schema.add_scalar_field(schema_definition::field_info(1,
      schema_definition::field_type::string, "string_field_2", "", false), m2);
  custom_schema.add_scalar_field(schema_definition::field_info(1,
      schema_definition::field_type::string, "string_field_nested", "", false),
      m3);

  custom_schema.add_message_field(schema_definition::field_info(2,
      schema_definition::field_type::message_type, "message_field",
      "second_message", false), m1);

  custom_schema.add_message_field(schema_definition::field_info(1,
      schema_definition::field_type::message_type, "repeated_msg_field",
      "second_message", true), m4);

  custom_schema.add_nested_message(m1, m3);
  custom_schema.add_message(m1);
  custom_schema.add_message(m2);
  custom_schema.add_message(m4);

  protobuf_schema sc;
  sc.import_schema_definition(&custom_schema, "test", "");

  int id1 = sc.new_message(sc.get_schema_id("first_message"));
  int id2 = sc.new_message(sc.get_schema_id("second_message"));
  int id3 = sc.new_message(sc.get_schema_id("first_message.nested_message"));

  int id4 = sc.new_message(sc.get_schema_id("first_message"));
  int id5 = sc.new_message(sc.get_schema_id("second_message"));
  int id6 = sc.new_message(sc.get_schema_id("first_message.nested_message"));

  sc.set_string(id1, 1, "value1");
  sc.set_string(id2, 1, "value2");
  sc.set_string(id3, 1, "value_nested");
  sc.set_message(id1, 2, id2);

  std::cout << sc.to_string(id1) << std::endl <<
              sc.to_string(id2) << std::endl <<
              sc.to_string(id3) << std::endl;

  EXPECT_EQ(sc.get_string(id3, 1), "value_nested");
  EXPECT_EQ(sc.get_string(id1, 1), "value1");
  EXPECT_EQ(sc.get_string(id2, 1), "value2");
  EXPECT_EQ(sc.get_string(sc.get_message(id1, 2), 1), "value2");

  std::string data1, data2, data3;

  sc.serialize_message(id1, &data1);
  sc.deserialize_message(id4, data1);
  sc.serialize_message(id2, &data2);
  sc.deserialize_message(id5, data2);
  sc.serialize_message(id3, &data3);
  sc.deserialize_message(id6, data3);

  EXPECT_EQ(sc.get_string(id6, 1), "value_nested");
  EXPECT_EQ(sc.get_string(id4, 1), "value1");
  EXPECT_EQ(sc.get_string(id5, 1), "value2");
  EXPECT_EQ(sc.get_string(sc.get_message(id4, 2), 1), "value2");

  int id7 = sc.new_message(sc.get_schema_id("second_message"));
  int id8 = sc.new_message(sc.get_schema_id("second_message"));
  int id9 = sc.new_message(sc.get_schema_id("third_message"));
  sc.set_string(id7, 1, "valueA");
  sc.set_string(id8, 1, "valueB");
  sc.set_repeated_message(id9, 1, id7, -1);
  sc.set_repeated_message(id9, 1, id8, -1);

  EXPECT_EQ(sc.get_repeated_field_size(id9, 1), 2);
  int id10 = sc.get_repeated_message(id9, 1, 0);
  int id11 = sc.get_repeated_message(id9, 1, 1);
  EXPECT_EQ(sc.get_string(id10, 1), "valueA");
  EXPECT_EQ(sc.get_string(id11, 1), "valueB");

  std::string data;
  int id13 = sc.new_message(sc.get_schema_id("third_message"));
  sc.serialize_message(id9, &data);
  sc.deserialize_message(id13, data);

  EXPECT_EQ(sc.get_repeated_field_size(id13, 1), 2);
  int id14 = sc.get_repeated_message(id13, 1, 0);
  int id15 = sc.get_repeated_message(id13, 1, 1);
  EXPECT_EQ(sc.get_string(id14, 1), "valueA");
  EXPECT_EQ(sc.get_string(id15, 1), "valueB");

  EXPECT_EQ(sc.get_field_type(sc.get_message_schema_id(id13), 1), "message");

  schema_definition::field_info field =
      sc.get_field_info(sc.get_schema_id("first_message"), 0);
  EXPECT_EQ(field.tag, 1);
  EXPECT_EQ(field.type, schema_definition::field_type::string);
  EXPECT_EQ(field.name, "string_field_1");
  EXPECT_EQ(field.fully_qualified_type, "");
  EXPECT_EQ(field.is_repeated, false);

  field = sc.get_field_info(sc.get_schema_id("first_message"), 1);
  EXPECT_EQ(field.tag, 2);
  EXPECT_EQ(field.type, schema_definition::field_type::message_type);
  EXPECT_EQ(field.name, "message_field");
  EXPECT_EQ(field.fully_qualified_type, "second_message");
  EXPECT_EQ(field.is_repeated, false);

  field = sc.get_field_info(sc.get_schema_id("third_message"), 0);
  EXPECT_EQ(field.tag, 1);
  EXPECT_EQ(field.type, schema_definition::field_type::message_type);
  EXPECT_EQ(field.name, "repeated_msg_field");
  EXPECT_EQ(field.fully_qualified_type, "second_message");
  EXPECT_EQ(field.is_repeated, true);

  // delete id13 and test if creating new message will get that spot (testing
  // insert_message())
  sc.delete_message(id13);
  int new_id = sc.new_message(0);
  EXPECT_EQ(new_id, id13);

  std::cout << "GOT TO THE END" << std::endl;
  google::protobuf::ShutdownProtobufLibrary();
}
