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

std::string string_to_hex(const std::string& input) {
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i) {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

}  // namespace io
}  // namespace core
}  // namespace automaton
