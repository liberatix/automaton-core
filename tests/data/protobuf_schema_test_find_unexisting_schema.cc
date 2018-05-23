#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "data/protobuf/protobuf_factory.h"
#include "gtest/gtest.h"

using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::io::get_file_contents;

int main(int argc, char* argv[]) {
  try {
    protobuf_factory sc;
    sc.import_schema_from_string(
        get_file_contents("tests/data/test.proto"), "test", "");
    sc.get_schema_id("ALABALA");
    google::protobuf::ShutdownProtobufLibrary();
    return 1;
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    google::protobuf::ShutdownProtobufLibrary();
  }
}
