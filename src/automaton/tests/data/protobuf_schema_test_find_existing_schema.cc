#include <fstream>
#include <iostream>
#include <string>

#include "automaton/core/io/io.h"
#include "automaton/core/data/protobuf/protobuf_factory.h"
#include "gtest/gtest.h"

using automaton::core::data::protobuf::protobuf_factory;
using automaton::core::io::get_file_contents;

int main(int argc, char* argv[]) {
  try {
    protobuf_factory sc;
    sc.import_schema_from_string(
        get_file_contents("automaton/tests/data/test.proto"), "test", "");
    int k = sc.get_schema_id("TestMsg");
    std::cout << "GOT: " << k << std::endl;
    google::protobuf::ShutdownProtobufLibrary();
    if (k != 0) {
      return k;
    } else {
      return 0;
    }
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    google::protobuf::ShutdownProtobufLibrary();
    return 200;
  }
  google::protobuf::ShutdownProtobufLibrary();
  return 100;
}
