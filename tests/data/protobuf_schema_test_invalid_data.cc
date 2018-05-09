#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "data/protobuf_schema.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
  try {
    protobuf_schema sc;
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
