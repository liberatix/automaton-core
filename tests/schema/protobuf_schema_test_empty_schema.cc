#include <fstream>
#include <iostream>
#include <string>

#include "schema/protobuf_schema.h"
#include "schema/protobuf_schema_definition.h"
#include "gtest/gtest.h"

std::string file_to_bytes(char const* filename) {
  std::ifstream is(filename, std::ifstream::binary);
  if (is) {
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    char* buffer = new char[length];
    std::cout << "Reading " << length << " characters... ";
    is.read(buffer, length);
    if (is) {
      std::cout << "all characters read successfully." << std::endl;
    } else {
      std::cout << "error: only " << is.gcount() << " could be read"
          << std::endl;
    }
    is.close();
    return std::string(buffer, length);
  }
  return "";
}

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
