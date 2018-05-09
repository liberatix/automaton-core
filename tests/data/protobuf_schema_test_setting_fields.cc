#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "data/protobuf/protobuf_factory.h"
#include "data/protobuf/protobuf_schema_message.h"
#include "gtest/gtest.h"

const char * TEST_MSG = "TestMsg";

TEST(protobuf_factory, setting_fields) {
  std::cout << "0" << std::endl;

  protobuf_factory sc;

  std::cout << "0a" << std::endl;

  sc.import_schema_from_string(
      get_file_contents("tests/data/test.proto"), "test", "");

  // *** String functions ***

  std::cout << "1" << std::endl;

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

  std::cout << "2" << std::endl;

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

  std::cout << "3" << std::endl;

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

  std::cout << "4" << std::endl;

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
