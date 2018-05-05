#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"

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

int main(int argc, char* argv[]) {
  try {
    protobuf_schema_definition custom_schema;
    int m1 = custom_schema.create_message("MyMessage");
    custom_schema.add_scalar_field(schema_definition::field_info(1,
        schema_definition::field_type::string, "string_field", "", false), m1);
    custom_schema.add_message_field(
        schema_definition::field_info(
            2, schema_definition::field_type::message_type,
            "message_field", "pack1.TestMsg", false),
        m1);
    custom_schema.add_message_field(
        schema_definition::field_info(
            3, schema_definition::field_type::message_type,
            "message_field2", "pack2.TestMsg2", false),
        m1);
    custom_schema.add_message(m1);
    custom_schema.add_dependency("name1");
    custom_schema.add_dependency("name2");

    protobuf_schema_definition custom_schema2;
    int m2 = custom_schema2.create_message("TestMsg2");
    custom_schema2.add_scalar_field(schema_definition::field_info(1,
        schema_definition::field_type::string, "string_field", "", false), m1);
    custom_schema2.add_message(m2);

    protobuf_schema sc;
    sc.import_schema_from_string(file_to_bytes("tests/data/test.proto"),
      "name1", "pack1");
    sc.import_schema_definition(&custom_schema2, "name2", "pack2");
    sc.import_schema_definition(&custom_schema, "name3", "pack3");
    for (int i = 0; i < sc.get_schemas_number(); i++) {
        sc.dump_message_schema(i, std::cout);
    }
    /*
      int schema_id = sc.get_schema_id("pack3.MyMessage");
      std::string msg = sc.get_message_field_type(schema_id,
          sc.get_field_tag(schema_id, "message_field2"));
      int a = sc.new_message(sc.get_schema_id(msg));
      sc.set_string(a,1,"alabala");
      std::cout << sc.to_string(a) << std::endl;
      int b = sc.new_message(schema_id);
      sc.set_message(b,3,a);
      std::cout << sc.to_string(b) << std::endl;
  */

    int k; std::cin >> k;
  } catch(std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
