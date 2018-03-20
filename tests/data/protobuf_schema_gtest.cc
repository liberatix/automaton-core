#include <fstream>
#include <iostream>
#include <string>

#include "data/protobuf_schema.h"
#include "gtest/gtest.h"

std::string file_to_bytes(char const* filename) {
  std::ifstream is(filename, std::ifstream::binary);
  if (is) {
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    char* buffer = new char[length];
    std::cout << "Reading " << length << " characters... ";
    // read data as a block:
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
  // Error reading proto file
}

TEST(data_protobuf, messages) {
  protobuf_schema_definition custom_schema;
  int m1 = custom_schema.create_message("MyMessage");
  custom_schema.add_scalar_field(schema::field_info(1,
      schema::field_type::string, "string_field", "", false), m1);
  custom_schema.add_message_field(schema::field_info(2,
      schema::field_type::message_type, "message_field", "pack1.TestMsg",
      false), m1);
  custom_schema.add_message_field(schema::field_info(3,
      schema::field_type::message_type, "message_field2", "TestMsg2", false),
      m1);
  custom_schema.add_message(m1);
  custom_schema.add_dependency("name1");
  custom_schema.add_dependency("name2");

  protobuf_schema_definition custom_schema2;
  int m2 = custom_schema2.create_message("TestMsg2");
  custom_schema2.add_scalar_field(schema::field_info(1,
      schema::field_type::string, "string_field", "", false), m1);
  custom_schema2.add_message(m2);

  protobuf_schema_definition custom_schema3;
  int m3 = custom_schema3.create_message("TestMsg");
  custom_schema3.add_scalar_field(schema::field_info(1,
      schema::field_type::string, "string_field", "", false), m1);
  custom_schema3.add_message(m3);

  protobuf_schema sc;
  sc.import_schema_definition(&custom_schema3, "name1", "pack1");
  sc.import_schema_definition(&custom_schema2, "name2", "pack2");
  sc.import_schema_definition(&custom_schema, "name3", "pack2");
  google::protobuf::ShutdownProtobufLibrary();
}
