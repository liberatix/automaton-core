#ifndef AUTOMATON_CORE_IO_IO_H__
#define AUTOMATON_CORE_IO_IO_H__

#include <string>

namespace automaton {
namespace core {
namespace io {

/**
  Gets the file contents and returns them as a string.

  Exception is thrown when error occurs.
*/
std::string get_file_contents(const char* filename);
std::string string_to_hex(const std::string& input);

}  // namespace io
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_IO_IO_H__
