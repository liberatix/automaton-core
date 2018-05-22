#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "data/protobuf/protobuf_factory.h"
#include "gtest/gtest.h"

using automaton::core::data::protobuf::protobuf_factory;

int main(int argc, char* argv[]) {
  try {
    protobuf_factory sc;
    sc.import_schema_from_string(
        get_file_contents("tests/data/invalid_data.proto"), "test", "");
  }
  catch (std::runtime_error& e) {
    // std::cout << e.what() << std::endl;
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
  }
  google::protobuf::ShutdownProtobufLibrary();
  return 1;
}
