#include <fstream>
#include <iostream>
#include <string>

#include "automaton/core/io/io.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"
#include "automaton/core/data/protobuf/protobuf_schema.h"
#include "gtest/gtest.h"

using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::data::protobuf::protobuf_schema;
using automaton::core::io::get_file_contents;

int main(int argc, char* argv[]) {
  try {
    protobuf_factory pb_factory;
    protobuf_schema loaded_schema(get_file_contents("automaton/tests/data/test.proto"));
    pb_factory.import_schema(&loaded_schema, "test", "");
    pb_factory.get_schema_id("ALABALA");
    google::protobuf::ShutdownProtobufLibrary();
    return 1;
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    google::protobuf::ShutdownProtobufLibrary();
  }
}
