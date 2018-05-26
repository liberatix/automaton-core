#include "automaton/core/script/registry.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_set>

#include "automaton/core/data/protobuf/protobuf_factory.h"
#include "automaton/core/log/log.h"

using automaton::core::data::factory;
using automaton::core::data::protobuf::protobuf_factory;

namespace automaton {
namespace core {
namespace script {

registry::registry() {
  auto ptr = new protobuf_factory();
  LOG(INFO) << "Created factory " << ptr;
  factory_ = std::unique_ptr<factory>(ptr);
}

std::string registry::to_string() {
  const size_t w1 = 25;
  const size_t w2 = 25;
  const size_t w3 = 30;
  const size_t w = w1 + w2 + w3;

  std::stringstream ss;
  ss << std::left;
  ss << std::string(w + 7, '=') << std::endl;
  ss << std::setw(w + 6)
     << "= automaton::core::script::registry modules" << "=" << std::endl;
  ss << std::string(w + 7, '=') << std::endl;
  ss << "| " << std::setw(w1) << "module name"
     << "| " << std::setw(w2) << "API + deps"
     << "| " << std::setw(w3) << "full version" << "|" << std::endl;
  ss << "+-" << std::string(w1, '-')
     << "+-" << std::string(w2, '-')
     << "+-" << std::string(w3, '-')
     << "+" << std::endl;
  std::vector<std::string> keys;
  for (const auto& m : modules_) {
    keys.push_back(m.first);
  }
  std::sort(keys.begin(), keys.end());
  for (const auto& k : keys) {
    const auto& m = *(modules_[k]);
    ss << "| " << std::setw(w1) << m.name()
       << "| " << std::setw(w2) << m.name_with_api_version()
       << "| " << std::setw(w3) << m.full_version() << "|" << std::endl;
    const auto& deps = m.dependencies();
    if (deps.size() > 0) {
      for (const auto& dep : deps) {
        ss << "| " << std::setw(w1) << ""
           << "| " << std::setw(w2) << (" - " + dep)
           << "| " << std::setw(w3) << "" << "|" << std::endl;
      }
    }
  }
  ss << "+-" << std::string(w1, '-')
     << "+-" << std::string(w2, '-')
     << "+-" << std::string(w3, '-')
     << "+" << std::endl;
  return ss.str();
}


}  // namespace script
}  // namespace core
}  // namespace automaton
