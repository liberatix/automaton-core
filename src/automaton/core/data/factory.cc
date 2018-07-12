#include "automaton/core/data/factory.h"

namespace automaton {
namespace core {
namespace data {

factory::~factory() {}

std::map<std::string, factory::data_factory_function> factory::schema_factory;

void factory::register_factory(std::string name, factory::data_factory_function func) {
  schema_factory[name] = func;
}

factory* factory::create(std::string name) {
  auto it = schema_factory.find(name);
  if (it == schema_factory.end()) {
    return nullptr;
  } else {
    return it->second();
  }
}

common::status factory::process(const obj& request, obj* response) {
  return common::status(common::status::OK);
}

}  // namespace data
}  // namespace core
}  // namespace automaton
