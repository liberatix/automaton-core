#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"
#include "schema/protobuf_schema_definition.h"
#include "gtest/gtest.h"

const char* FIRST_MESSAGE = "first_message";
const char* SECOND_MESSAGE = "second_message";
const char* THIRD_MESSAGE = "third_message";
const char* NESTED_MESSAGE = "nested_message";
const char* FIRST_MESSAGE_NESTED_MESSAGE = "first_message.nested_message";

const char* STRING_FIELD_1 = "string_field_1";
const char* STRING_FIELD_2 = "string_field_2";
const char* STRING_FIELD_NESTED = "string_field_nested";
const char* MESSAGE_FIELD = "message_field";
const char* REPEATED_MSG_FIELD = "repeated_msg_field";

const char* VALUE_1 = "value_1";
const char* VALUE_2 = "value_2";
const char* VALUE_A = "value_A";
const char* VALUE_B = "value_B";
const char* VALUE_NESTED = "value_nested";

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

TEST(protobuf_schema_message, messages) {
  protobuf_schema_definition* csp = new protobuf_schema_definition();
  protobuf_schema_definition& custom_schema = *csp;
  int m1 = custom_schema.create_message(FIRST_MESSAGE);
  int m2 = custom_schema.create_message(SECOND_MESSAGE);
  int m3 = custom_schema.create_message(NESTED_MESSAGE);
  int m4 = custom_schema.create_message(THIRD_MESSAGE);

  custom_schema.add_scalar_field(schema_definition::field_info(1,
      schema_definition::string, STRING_FIELD_1, "", false), m1);
  custom_schema.add_scalar_field(schema_definition::field_info(1,
      schema_definition::string, STRING_FIELD_2, "", false), m2);
  custom_schema.add_scalar_field(schema_definition::field_info(1,
      schema_definition::string, STRING_FIELD_NESTED, "", false), m3);

  custom_schema.add_message_field(schema_definition::field_info(2,
      schema_definition::message_type, MESSAGE_FIELD,
      SECOND_MESSAGE, false), m1);

  custom_schema.add_message_field(schema_definition::field_info(1,
      schema_definition::message_type, REPEATED_MSG_FIELD,
      SECOND_MESSAGE, true), m4);

  custom_schema.add_nested_message(m1, m3);
  custom_schema.add_message(m1);
  custom_schema.add_message(m2);
  custom_schema.add_message(m4);

  protobuf_schema* scp = new protobuf_schema();
  protobuf_schema& sc = *scp;
  sc.import_schema_definition(&custom_schema, "test", "");

  schema_message* msg1 = sc.new_message(FIRST_MESSAGE);
  schema_message* msg2 = sc.new_message(SECOND_MESSAGE);
  schema_message* msg3 = sc.new_message(FIRST_MESSAGE_NESTED_MESSAGE);

  schema_message* msg4 = sc.new_message(FIRST_MESSAGE);
  schema_message* msg5 = sc.new_message(SECOND_MESSAGE);
  schema_message* msg6 = sc.new_message(FIRST_MESSAGE_NESTED_MESSAGE);

  msg1->set_string(1, VALUE_1);
  msg2->set_string(1, VALUE_2);
  msg3->set_string(1, VALUE_NESTED);
  msg1->set_message(2, msg2);

  std::cout << "MSG1: " << msg1->to_string() << std::endl <<
               "MSG2: " << msg2->to_string() << std::endl <<
               "MSG3: " << msg3->to_string() << std::endl;

  EXPECT_EQ(msg3->get_string(1), VALUE_NESTED);
  EXPECT_EQ(msg1->get_string(1), VALUE_1);
  EXPECT_EQ(msg2->get_string(1), VALUE_2);
  EXPECT_EQ(msg1->get_message(2)->get_string(1), VALUE_2);

  std::string data;

  msg1->serialize_message(&data);
  msg4->deserialize_message(data);
  msg2->serialize_message(&data);
  msg5->deserialize_message(data);
  msg3->serialize_message(&data);
  msg6->deserialize_message(data);

  EXPECT_EQ(msg4->get_string(1), VALUE_1);
  EXPECT_EQ(msg4->get_message(2)->get_string(1), VALUE_2);
  EXPECT_EQ(msg5->get_string(1), VALUE_2);
  EXPECT_EQ(msg6->get_string(1), VALUE_NESTED);

  schema_message* msg7 = sc.new_message(SECOND_MESSAGE);
  schema_message* msg8 = sc.new_message(SECOND_MESSAGE);
  schema_message* msg9 = sc.new_message(THIRD_MESSAGE);

  msg7->set_string(1, VALUE_A);
  msg8->set_string(1, VALUE_B);
  msg9->set_repeated_message(1, msg7, -1);
  msg9->set_repeated_message(1, msg8, -1);

  EXPECT_EQ(msg9->get_repeated_field_size(1), 2);
  schema_message* msg10 = msg9->get_repeated_message(1, 0);
  schema_message* msg11 = msg9->get_repeated_message(1, 1);
  EXPECT_EQ(msg10->get_string(1), VALUE_A);
  EXPECT_EQ(msg11->get_string(1), VALUE_B);

  schema_message* msg13 = sc.new_message(THIRD_MESSAGE);
  msg9->serialize_message(&data);
  msg13->deserialize_message(data);

  EXPECT_EQ(msg13->get_repeated_field_size(1), 2);
  schema_message* msg14 = msg13->get_repeated_message(1, 0);
  schema_message* msg15 = msg13->get_repeated_message(1, 1);
  EXPECT_EQ(msg14->get_string(1), VALUE_A);
  EXPECT_EQ(msg15->get_string(1), VALUE_B);

  EXPECT_EQ(
      sc.get_field_type(
          sc.get_schema_id(msg13->get_message_type()),
          1),
      "message");

  schema_definition::field_info field =
      sc.get_field_info(sc.get_schema_id(FIRST_MESSAGE), 0);
  EXPECT_EQ(field.tag, 1);
  EXPECT_EQ(field.type, schema_definition::field_type::string);
  EXPECT_EQ(field.name, STRING_FIELD_1);
  EXPECT_EQ(field.fully_qualified_type, "");
  EXPECT_EQ(field.is_repeated, false);

  field = sc.get_field_info(sc.get_schema_id(FIRST_MESSAGE), 1);
  EXPECT_EQ(field.tag, 2);
  EXPECT_EQ(field.type, schema_definition::field_type::message_type);
  EXPECT_EQ(field.name, MESSAGE_FIELD);
  EXPECT_EQ(field.fully_qualified_type, SECOND_MESSAGE);
  EXPECT_EQ(field.is_repeated, false);

  field = sc.get_field_info(sc.get_schema_id(THIRD_MESSAGE), 0);
  EXPECT_EQ(field.tag, 1);
  EXPECT_EQ(field.type, schema_definition::field_type::message_type);
  EXPECT_EQ(field.name, REPEATED_MSG_FIELD);
  EXPECT_EQ(field.fully_qualified_type, SECOND_MESSAGE);
  EXPECT_EQ(field.is_repeated, true);

  delete msg1;
  delete msg2;
  delete msg3;
  delete msg4;
  delete msg5;
  delete msg6;
  delete msg7;
  delete msg8;
  delete msg9;
  delete msg10;
  delete msg11;
  // delete msg12;
  delete msg13;
  delete msg14;
  delete msg15;

  delete scp;
  delete csp;

  google::protobuf::ShutdownProtobufLibrary();
}
