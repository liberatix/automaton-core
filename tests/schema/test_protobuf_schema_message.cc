#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"
#include "schema/protobuf_schema_definition.h"
#include "gtest/gtest.h"

TEST(protobuf_schema_message, messages) {
  /*
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
  */

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

  schema_message * msg1 = sc.new_message("first_message");
  schema_message * msg2 = sc.new_message("second_message");
  schema_message * msg3 = sc.new_message("first_message.nested_message");

  schema_message * msg4 = sc.new_message("first_message");
  schema_message * msg5 = sc.new_message("second_message");
  schema_message * msg6 = sc.new_message("first_message.nested_message");

  msg1->set_string(1, "value1");
  msg2->set_string(1, "value2");
  msg3->set_string(1, "value_nested");
  msg1->set_message(2, msg2);

  std::cout << "MSG1: " << msg1->to_string() << std::endl <<
               "MSG2: " << msg2->to_string() << std::endl <<
               "MSG3: " << msg3->to_string() << std::endl;

  EXPECT_EQ(msg3->get_string(1), "value_nested");
  EXPECT_EQ(msg1->get_string(1), "value1");
  EXPECT_EQ(msg2->get_string(1), "value2");
  EXPECT_EQ(msg1->get_message(2)->get_string(1), "value2");

  std::string data;

  msg1->serialize_message(&data);
  msg4->deserialize_message(data);
  msg2->serialize_message(&data);
  msg5->deserialize_message(data);
  msg3->serialize_message(&data);
  msg6->deserialize_message(data);

  EXPECT_EQ(msg4->get_string(1), "value1");
  EXPECT_EQ(msg4->get_message(2)->get_string(1), "value2");
  EXPECT_EQ(msg5->get_string(1), "value2");
  EXPECT_EQ(msg6->get_string(1), "value_nested");

  schema_message * msg7 = sc.new_message("second_message");
  schema_message * msg8 = sc.new_message("second_message");
  schema_message * msg9 = sc.new_message("third_message");

  msg7->set_string(1, "valueA");
  msg8->set_string(1, "valueB");
  msg9->set_repeated_message(1, msg7, -1);
  msg9->set_repeated_message(1, msg8, -1);

  EXPECT_EQ(msg9->get_repeated_field_size(1), 2);
  schema_message * msg10 = msg9->get_repeated_message(1, 0);
  schema_message * msg11 = msg9->get_repeated_message(1, 1);
  EXPECT_EQ(msg10->get_string(1), "valueA");
  EXPECT_EQ(msg11->get_string(1), "valueB");

  schema_message * msg13 = sc.new_message("third_message");
  msg9->serialize_message(&data);
  msg13->deserialize_message(data);

  EXPECT_EQ(msg13->get_repeated_field_size(1), 2);
  schema_message * msg14 = msg13->get_repeated_message(1, 0);
  schema_message * msg15 = msg13->get_repeated_message(1, 1);
  EXPECT_EQ(msg14->get_string(1), "valueA");
  EXPECT_EQ(msg15->get_string(1), "valueB");

  EXPECT_EQ(
      sc.get_field_type(
          sc.get_schema_id(msg13->get_message_type()),
          1),
      "message");

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

  std::cout << "GOT TO THE END" << std::endl;
  google::protobuf::ShutdownProtobufLibrary();
}
