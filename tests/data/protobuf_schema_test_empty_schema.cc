#include <fstream>
#include <iostream>
#include <string>

#include "data/protobuf/protobuf_factory.h"
#include "data/protobuf/protobuf_schema.h"
#include "gtest/gtest.h"

using data::protobuf::protobuf_factory;
using data::protobuf::protobuf_schema;

int main(int argc, char* argv[]) {
  try {
    protobuf_schema custom_schema;
    protobuf_factory sc;
    sc.import_schema_definition(&custom_schema, "test", "");
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    google::protobuf::ShutdownProtobufLibrary();
    return 1;
  }
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
