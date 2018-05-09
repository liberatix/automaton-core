#include <fstream>
#include <iostream>
#include <string>

#include "data/protobuf_schema.h"
#include "data/protobuf_schema_definition.h"
#include "gtest/gtest.h"

TEST(data_protobuf, messages) {
  protobuf_schema_definition custom_schema;
  int m1 = custom_schema.create_message("MyMessage");
  custom_schema.add_scalar_field(
      schema_definition::field_info(1,
          schema_definition::field_type::string, "string_field", "", false),
      m1);
  custom_schema.add_message_field(
      schema_definition::field_info(2,
          schema_definition::field_type::message_type, "message_field",
          "pack1.TestMsg", false),
      m1);
  custom_schema.add_message_field(
      schema_definition::field_info(3,
          schema_definition::field_type::message_type, "message_field2",
          "TestMsg2", false),
      m1);
  custom_schema.add_message(m1);
  custom_schema.add_dependency("name1");
  custom_schema.add_dependency("name2");

  protobuf_schema_definition custom_schema2;
  int m2 = custom_schema2.create_message("TestMsg2");
  custom_schema2.add_scalar_field(schema_definition::field_info(1,
      schema_definition::field_type::string, "string_field", "", false), m1);
  custom_schema2.add_message(m2);

  protobuf_schema_definition custom_schema3;
  int m3 = custom_schema3.create_message("TestMsg");
  custom_schema3.add_scalar_field(schema_definition::field_info(1,
      schema_definition::field_type::string, "string_field", "", false), m1);
  custom_schema3.add_message(m3);

  protobuf_schema sc;
  sc.import_schema_definition(&custom_schema3, "name1", "pack1");
  sc.import_schema_definition(&custom_schema2, "name2", "pack2");
  sc.import_schema_definition(&custom_schema, "name3", "pack2");
  google::protobuf::ShutdownProtobufLibrary();
}
