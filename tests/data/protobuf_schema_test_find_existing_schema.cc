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
        get_file_contents("tests/data/test.proto"), "test", "");
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
