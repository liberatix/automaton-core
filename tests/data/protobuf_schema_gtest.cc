#include <fstream>
#include <iostream>
#include <string>

#include "data/protobuf/protobuf_factory.h"
#include "data/protobuf/protobuf_schema.h"
#include "gtest/gtest.h"

using automaton::core::data::schema;
using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::data::protobuf::protobuf_schema;

TEST(data_protobuf, messages) {
  protobuf_schema cs;
  int m1 = cs.create_message("MyMessage");
  cs.add_scalar_field(
      schema::field_info(1, schema::string, "string_field", "", false), m1);
  cs.add_message_field(
      schema::field_info(
          2, schema::message_type, "message_field", "pack1.TestMsg", false),
      m1);
  cs.add_message_field(
      schema::field_info(
          3, schema::message_type, "message_field2", "TestMsg2", false),
      m1);
  cs.add_message(m1);
  cs.add_dependency("name1");
  cs.add_dependency("name2");

  protobuf_schema cs2;
  int m2 = cs2.create_message("TestMsg2");
  cs2.add_scalar_field(
      schema::field_info(1, schema::string, "string_field", "", false), m1);
  cs2.add_message(m2);

  protobuf_schema cs3;
  int m3 = cs3.create_message("TestMsg");
  cs3.add_scalar_field(
      schema::field_info(1, schema::string, "string_field", "", false), m1);
  cs3.add_message(m3);

  protobuf_factory sc;
  sc.import_schema_definition(&cs3, "name1", "pack1");
  sc.import_schema_definition(&cs2, "name2", "pack2");
  sc.import_schema_definition(&cs, "name3", "pack2");
  google::protobuf::ShutdownProtobufLibrary();
}
