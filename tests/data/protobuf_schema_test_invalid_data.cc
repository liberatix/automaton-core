#include <fstream>
#include <iostream>
#include <string>

#include "data/protobuf_schema.h"
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
    protobuf_schema sc;
    sc.import_schema_from_string(file_to_bytes("tests/data/invalid_data.proto"),
        "test", "");
  }
  catch (std::runtime_error& e) {
    // std::cout << e.what() << std::endl;
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
  }
  return 1;
}
