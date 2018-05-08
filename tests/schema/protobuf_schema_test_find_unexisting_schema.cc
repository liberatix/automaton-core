#include <fstream>
#include <iostream>
#include <string>

#include "io/io.h"
#include "schema/protobuf_schema.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
  try {
    protobuf_schema sc;
    sc.import_schema_from_string(
        get_file_contents("tests/schema/test.proto"), "test", "");
    sc.get_schema_id("ALABALA");
    google::protobuf::ShutdownProtobufLibrary();
    return 1;
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    google::protobuf::ShutdownProtobufLibrary();
  }
}
