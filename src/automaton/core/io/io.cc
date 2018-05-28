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

}  // namespace io
}  // namespace core
}  // namespace automaton
