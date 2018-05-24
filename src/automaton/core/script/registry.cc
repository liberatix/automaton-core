#include "automaton/core/script/registry.h"

#include <string>
#include <sstream>

namespace automaton {
namespace core {
namespace script {

std::string registry::to_string() {
  std::stringstream ss;
  ss << "-------------------------------------------" << std::endl;
  ss << " automaton::core::script::registry modules " << std::endl;
  ss << "-------------------------------------------" << std::endl;
  for (auto m : modules) {
    ss << "[" + m.second.version + "] " << m.first << std::endl;
  }
  ss << "-------------------------------------------" << std::endl;
  return ss.str();
}


}  // namespace script
}  // namespace core
}  // namespace automaton
