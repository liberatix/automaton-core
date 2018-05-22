#include "io/io.h"

#include <fstream>
#include <string>
#include <cerrno>

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
  throw(errno);
}

}  // namespace io
}  // namespace core
}  // namespace automaton
