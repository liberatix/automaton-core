#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"
#include "schema/protobuf_schema_definition.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
  try {
    protobuf_schema_definition custom_schema;
    protobuf_schema sc;
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
