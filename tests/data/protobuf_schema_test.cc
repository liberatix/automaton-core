#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "data/protobuf/protobuf_factory.h"
#include "data/protobuf/protobuf_schema_definition.h"

int main(int argc, char* argv[]) {
  try {
    protobuf_schema_definition custom_schema;
    int m1 = custom_schema.create_message("MyMessage");
    custom_schema.add_scalar_field(schema::field_info(1,
        schema::string, "string_field", "", false), m1);
    custom_schema.add_message_field(
        schema::field_info(
            2, schema::message_type,
            "message_field", "pack1.TestMsg", false),
        m1);
    custom_schema.add_message_field(
        schema::field_info(
            3, schema::message_type,
            "message_field2", "pack2.TestMsg2", false),
        m1);
    custom_schema.add_message(m1);
    custom_schema.add_dependency("name1");
    custom_schema.add_dependency("name2");

    protobuf_schema_definition custom_schema2;
    int m2 = custom_schema2.create_message("TestMsg2");
    custom_schema2.add_scalar_field(schema::field_info(1,
        schema::field_type::string, "string_field", "", false), m1);
    custom_schema2.add_message(m2);

    protobuf_factory sc;
    sc.import_schema_from_string(
        get_file_contents("tests/data/test.proto"), "name1", "pack1");
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
