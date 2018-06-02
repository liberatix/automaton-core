#include <fstream>
#include <iostream>
#include <string>

#include "automaton/core/io/io.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "gtest/gtest.h"

using automaton::core::data::msg;
using automaton::core::data::schema;
using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::io::get_file_contents;

TEST(protobuf_schema, schema_info_from_message) {
  protobuf_schema loaded_schema(get_file_contents("automaton/tests/data/many_fields.proto"));
  loaded_schema.dump_schema(std::cout);
  std::cout << "======================" << std::endl;
  protobuf_schema loaded_schema2(get_file_contents("automaton/tests/data/many_enums.proto"));
  loaded_schema2.dump_schema(std::cout);
  google::protobuf::ShutdownProtobufLibrary();
}
