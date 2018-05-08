#ifndef AUTOMATON_CORE_IO_IO_H__
#define AUTOMATON_CORE_IO_IO_H__

#include <string>

/**
  Gets the file contents and returns them as a string.
  
  Exception is thrown when error occurs.
*/
std::string get_file_contents(const char* filename);

#endif  // AUTOMATON_CORE_IO_IO_H__
