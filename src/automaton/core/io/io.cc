#include "automaton/core/io/io.h"

#include <fstream>
#include <string>
#include <cerrno>

#include "automaton/core/log/log.h"

namespace automaton {
namespace core {
namespace io {

std::string get_file_contents(const char* filename) {
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in) {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  LOG(ERROR) << "Could not read file contents for " << filename;
  throw(errno);
}

static const char* const hex_lut = "0123456789ABCDEF";

std::string bin2hex(const std::string& input) {
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i) {
        const unsigned char c = input[i];
        output.push_back(hex_lut[c >> 4]);
        output.push_back(hex_lut[c & 15]);
    }
    return output;
}

uint8_t hex2nibble(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  throw std::invalid_argument(std::to_string(c) + "not a hex digit!");
}

std::string hex2bin(const std::string& input) {
  size_t len = input.length();
  size_t out_len = (len + 1) / 2;

  std::string output;
  output.reserve(out_len);

  // If input string is an odd length use one nibble to produce an output byte.
  if (len & 1) {
    output.push_back(hex2nibble(input[0]));
  }

  // The rest of the input consists of pair of nibbles.
  for (size_t i = (len & 1); i < len-1; i+=2) {
    uint8_t n1 = hex2nibble(input[i]);
    uint8_t n2 = hex2nibble(input[i+1]);
    output.push_back(n1 << 4 | n2);
  }

  return output;
}


}  // namespace io
}  // namespace core
}  // namespace automaton
