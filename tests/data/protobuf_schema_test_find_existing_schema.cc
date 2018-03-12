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

TEST("protobufs_schema", "find_existing_schema") {
  try{
		protobuf_schema sc(FileToBytes("testing/protos/test.proto"),"test");
		int k = sc.GetSchemaID("TestMsg");
		if (k == 0)
			cout << "Finding existing message ... PASSED! " << endl;
		else{
			cout << "Finding existing message ... FAILED! " << endl;
			cout << "Expected 0 got " << k << endl;
		}
	}
	catch (exception& e){
		cout << "Finding existing message ... FAILED! " << endl;
		cout << e.what() << endl;
	}
  return 0;
}
