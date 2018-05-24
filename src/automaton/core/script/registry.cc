#include "automaton/core/script/registry.h"

#include <algorithm>
#include <string>
#include <sstream>

#include "automaton/core/data/protobuf/protobuf_factory.h"

using automaton::core::data::factory;
using automaton::core::data::protobuf::protobuf_factory;

namespace automaton {
namespace core {
namespace script {

registry::registry() {
  factory_ = std::unique_ptr<factory>(new protobuf_factory());
}

registry& registry::get() {
  static registry * instance = nullptr;
  if (instance == nullptr) {
    instance = new registry();
  }
  return *instance;
}

std::string registry::to_string() {
  std::stringstream ss;
  ss << "-------------------------------------------" << std::endl;
  ss << " automaton::core::script::registry modules " << std::endl;
  ss << "-------------------------------------------" << std::endl;
  std::vector<std::string> keys;
  for (const auto& m : modules_) {
    keys.push_back(m.first);
  }
  std::sort(keys.begin(), keys.end());
  for (const auto& k : keys) {
    const auto& m = modules_[k];
    ss << "[" + m.version + "] " << m.name << std::endl;
  }
  ss << "-------------------------------------------" << std::endl;
  return ss.str();
}


}  // namespace script
}  // namespace core
}  // namespace automaton
