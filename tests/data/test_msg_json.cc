#include <fstream>
#include <iostream>
#include <string>

#include "data/protobuf/protobuf_factory.h"
#include "data/protobuf/protobuf_schema.h"
#include "gtest/gtest.h"

using data::schema;
using data::msg;
using data::protobuf::protobuf_factory;
using data::protobuf::protobuf_schema;


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
const char* REPEATED_STRING_FIELD = "repeated_string_field";

const char* VALUE_1 = "value_1";
const char* VALUE_2 = "value_2";
const char* VALUE_A = "value_A";
const char* VALUE_B = "value_B";
const char* VALUE_NESTED = "value_nested";

class test_msg_json : public ::testing::Test {
 protected:
  // You can define per-test set-up and tear-down logic as usual.
  virtual void SetUp() {
    ps.reset(new protobuf_schema());
    pf.reset(new protobuf_factory());
    setup_schema();
  }

  virtual void TearDown() {
    ps.release();
    pf.release();
  }

  /*
    first_message {
      string string_field_1 = 1;
      second_message message_field = 2;
      repeated string repeated_string_field = 3;

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
  void setup_schema() {
    int m1 = ps->create_message(FIRST_MESSAGE);
    int m2 = ps->create_message(SECOND_MESSAGE);
    int m3 = ps->create_message(NESTED_MESSAGE);
    int m4 = ps->create_message(THIRD_MESSAGE);

    ps->add_scalar_field(
        schema::field_info(1, schema::string, STRING_FIELD_1, "", false), m1);
    ps->add_scalar_field(
        schema::field_info(1, schema::string, STRING_FIELD_2, "", false), m2);
    ps->add_scalar_field(
        schema::field_info(1, schema::string, STRING_FIELD_NESTED, "", false),
        m3);

    ps->add_message_field(schema::field_info(2,
        schema::message_type, MESSAGE_FIELD,
        SECOND_MESSAGE, false), m1);

    ps->add_scalar_field(
        schema::field_info(3, schema::string, REPEATED_STRING_FIELD, "", true), m1);

    ps->add_message_field(schema::field_info(1,
        schema::message_type, REPEATED_MSG_FIELD,
        SECOND_MESSAGE, true), m4);

    ps->add_nested_message(m1, m3);
    ps->add_message(m1);
    ps->add_message(m2);
    ps->add_message(m4);
  }

  static std::unique_ptr<protobuf_schema> ps;
  static std::unique_ptr<protobuf_factory> pf;
};

std::unique_ptr<protobuf_schema> test_msg_json::ps;
std::unique_ptr<protobuf_factory> test_msg_json::pf;

TEST_F(test_msg_json, serialize_json) {
  pf->import_schema_definition(ps.get(), "test", "");

  auto msg1 = pf->new_message(FIRST_MESSAGE);
  auto msg2 = pf->new_message(SECOND_MESSAGE);
  auto msg3 = pf->new_message(FIRST_MESSAGE_NESTED_MESSAGE);

  msg1->set_string(1, VALUE_1);

  msg2->set_string(1, VALUE_2);
  msg1->set_message(2, *msg2);
  msg1->set_repeated_string(3, "R1", -1);
  msg1->set_repeated_string(3, "R2", -1);
  msg1->set_repeated_string(3, "R3", -1);

  // Deserialize to JSON.
  std::string json;
  msg1->to_json(&json);
  std::cout << json << std::endl;

  // Serialize from JSON.
  auto msg4 = pf->new_message(FIRST_MESSAGE);
  msg4->from_json(json);
  std::cout << "DESERIALIZED MSG: " << msg4->to_string() << std::endl;

  google::protobuf::ShutdownProtobufLibrary();
}
