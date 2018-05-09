#include "data/factory.h"

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
