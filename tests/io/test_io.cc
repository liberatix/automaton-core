#include "io/io.h"
#include "gtest/gtest.h"

using automaton::core::io::get_file_contents;

const char * TEST_TXT_CONTENTS = "Hello, World!";

TEST(io, get_file_contents) {
  std::string test_txt = get_file_contents("tests/io/test.txt");
  EXPECT_EQ(test_txt, TEST_TXT_CONTENTS);
}
