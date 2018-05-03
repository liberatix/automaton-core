#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"
#include "gtest/gtest.h"

std::string file_to_bytes(char const* filename) {
  std::ifstream is(filename, std::ifstream::binary);
  if (is) {
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    char* buffer = new char[length];
    std::cout << "Reading " << length << " characters... ";
    is.read(buffer, length);
    if (is) {
      std::cout << "all characters read successfully." << std::endl;
    } else {
      std::cout << "error: only " << is.gcount() << " could be read"
          << std::endl;
    }
    is.close();
    return std::string(buffer, length);
  }
  return "";
}
TEST(protobuf_schema, setting_fields) {
  protobuf_schema sc;
  sc.import_schema_from_string(file_to_bytes("tests/schema/test.proto"),
      "test", "");

  /**
    String functions
  **/

  // No such field
  try {
    int id = sc.new_message(0);
    sc.set_string(id, 100, "value");
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No field with tag: 100");
  }

  // No such message
  try {
    sc.set_string(-5, 1, "value");
  }
  catch (std::out_of_range& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No message with id: -5");
  }

  // Data field is repeated
  try {
    int id = sc.new_message(0);
    sc.set_string(1, 4, "value");
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is repeated");
  }

  // Field is not string
  try {
    int id = sc.new_message(0);
    sc.set_string(id, 3, "value");
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
    int id = sc.new_message(0);
    sc.set_repeated_string(id, 100, "value", -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No field with tag: 100");
  }
  // No such message
  try {
    sc.set_repeated_string(-5, 1, "value", -1);
  }
  catch (std::out_of_range& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No message with id: -5");
  }
  // Data field is not repeated
  try {
    int id = sc.new_message(0);
    sc.set_repeated_string(id, 1, "value", -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not repeated");
  }
  // Field is not string
  try {
    int id = sc.new_message(0);
    sc.set_repeated_string(id, 3, "value", -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not string");
  }


  /**
    Int32 functions
  **/

  // No such field
  try {
    int id = sc.new_message(0);
    sc.set_int32(id, 100, 42);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No field with tag: 100");
  }
  // No such message
  try {
    sc.set_int32(-5, 5, 42);
  }
  catch (std::out_of_range& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No message with id: -5");
  }
  // Data field is repeated
  try {
    int id = sc.new_message(0);
    sc.set_int32(id, 7, 42);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is repeated");
  }
  // Field is not int32
  try {
    int id = sc.new_message(0);
    sc.set_int32(id, 1, 42);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not int32");
  }

  /**
    Int32 array functions
  **/

  // No such field
  try {
    int id = sc.new_message(0);
    sc.set_repeated_int32(id, 100, 42, -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No field with tag: 100");
  }
  // No such message
  try {
    sc.set_repeated_int32(-5, 1, 42, -1);
  }
  catch (std::out_of_range& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "No message with id: -5");
  }
  // Data field is not repeated
  try {
    int id = sc.new_message(0);
    sc.set_repeated_int32(id, 2, 42, -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not repeated");
  }
  // Field is not int32
  try {
    int id = sc.new_message(0);
    sc.set_repeated_int32(id, 4, 42, -1);
  }
  catch (std::invalid_argument& e) {
    std::string message = e.what();
    std::cerr << message << std::endl;
    EXPECT_EQ(message, "Field is not int32");
  }
  google::protobuf::ShutdownProtobufLibrary();
}
