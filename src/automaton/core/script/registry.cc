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
  ss << std::endl;
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
  ss << std::endl;

  // Show all concepts, interfaces and functions.
  for (const auto& k : keys) {
    const size_t w1 = 20;
    const size_t w2 = 40;
    const size_t w = w1 + w2;
    const auto& m = *(modules_[k]);
    ss << std::string(w + 5, '=') << std::endl;
    ss << std::setw(w + 4)
       << (std::string("= module ") + m.name_with_api_version()) << "=" << std::endl;
    ss << std::string(w + 5, '=') << std::endl;

    // functions
    bool first_line = true;
    for (auto function : m.functions()) {
      ss << "| " << std::setw(w1) << (first_line ? "functions" : "")
         << "| " << std::setw(w2) << function
         << "|" << std::endl;
      first_line = false;
    }
    if (!first_line) {
      ss << "+-" << std::string(w1, '-')
         << "+-" << std::string(w2, '-')
         << "+" << std::endl;
    }

    // concepts
    first_line = true;
    for (auto concept : m.concepts()) {
      ss << "| " << std::setw(w1) << (first_line ? "concepts" : "")
         << "| " << std::setw(w2) << concept
         << "|" << std::endl;
      first_line = false;
    }
    if (!first_line) {
      ss << "+-" << std::string(w1, '-')
         << "+-" << std::string(w2, '-')
         << "+" << std::endl;
    }

    // implementations
    first_line = true;
    for (const auto& kv : m.implementations()) {
      const auto& implementation = kv.first;
      std::stringstream impl_info;
      impl_info << implementation << " (";

      // List implemented concepts.
      auto schema_id = factory_->get_schema_id(m.name_with_api_version() + "." + implementation);
      auto num_fields = factory_->get_fields_number(schema_id);
      for (size_t i = 0; i < num_fields; ++i) {
        auto field_info = factory_->get_field_info(schema_id, i);
        if (i > 0) {
          impl_info << ", ";
        }
        impl_info << field_info.fully_qualified_type;
      }
      impl_info << ")";

      ss << "| " << std::setw(w1) << (first_line ? "implementations" : "")
         << "| " << std::setw(w2) << impl_info.str()
         << "|" << std::endl;

      first_line = false;
    }
    if (!first_line) {
      ss << "+-" << std::string(w1, '-')
         << "+-" << std::string(w2, '-')
         << "+" << std::endl;
    }
    ss << std::endl;
  }

/*
  int msg_schemas_number = factory_->get_schemas_number();
  for (size_t i = 0; i < msg_schemas_number; ++i) {
    ss << factory_->get_schema_name(i) << std::endl;
  }
*/
  return ss.str();
}

common::obj* registry::create(const data::msg& m) {
  LOG(INFO) << "Creating object of type " << m.get_message_type();

  // Route message to module factory.
  static const boost::regex obj_regex{"^([^\\.]+\\.v\\d*)\\.(.*)$"};
  boost::smatch what;
  if (boost::regex_search(m.get_message_type(), what, obj_regex)) {
    auto module_name = what[1].str();
    auto object_type = what[2].str();
    LOG(INFO) << "module: " << module_name;
    LOG(INFO) << "object type: " << object_type;
    if (modules_.count(module_name) == 0) {
      std::stringstream msg;
      msg << "No such module " << module_name;
      LOG(ERROR) << msg.str();
      throw std::invalid_argument(msg.str());
    }
    auto& mod = modules_.at(module_name);
    auto& impls = mod->implementations();
    if (impls.count(object_type) == 0) {
      std::stringstream msg;
      msg << "No factory found for object type " << object_type << " in module " << module_name;
      LOG(ERROR) << msg.str();
      throw std::invalid_argument(msg.str());
    }
    return impls.at(object_type)(m);
  } else {
    // TODO(asen): Trhow an actual exception.
    throw "Invalid creation message!";
  }
}


}  // namespace script
}  // namespace core
}  // namespace automaton
