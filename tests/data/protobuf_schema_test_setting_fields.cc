#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "data/protobuf_schema.h"
#include "data/protobuf_schema_message.h"
#include "gtest/gtest.h"

const char * TEST_MSG = "TestMsg";

TEST(protobuf_schema, setting_fields) {
  protobuf_schema sc;
  sc.import_schema_from_string(
      get_file_contents("tests/schema/test.proto"), "test", "");

  // *** String functions ***

  // No such field
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_string(100, "value");
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No field with tag: 100");
  }

  // Data field is repeated
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_string(4, "value");
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is repeated");
  }

  // Field is not string
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_string(3, "value");
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not string");
  }

  /**
    String array functions
  **/

  // No such field
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_repeated_string(100, "value", -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No field with tag: 100");
  }

  // Data field is not repeated
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_repeated_string(1, "value", -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not repeated");
  }

  // Field is not string
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_repeated_string(3, "value", -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not string");
  }

  // *** Int32 functions ***

  // No such field
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_int32(100, 42);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No field with tag: 100");
  }

  // Data field is repeated
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_int32(7, 42);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is repeated");
  }

  // Field is not int32
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_int32(1, 42);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not int32");
  }

  // *** Int32 array functions ***

  // No such field
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_repeated_int32(100, 42, -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No field with tag: 100");
  }

  // Data field is not repeated
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_repeated_int32(2, 42, -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not repeated");
  }

  // Field is not int32
  try {
    msg * msg = sc.new_message(TEST_MSG);
    msg->set_repeated_int32(4, 42, -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not int32");
  }
  google::protobuf::ShutdownProtobufLibrary();
}
