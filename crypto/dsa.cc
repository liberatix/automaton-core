#include "crypto/dsa.h"

std::map<std::string, dsa::dsa_factory_function> dsa::dsa_factory;

dsa * dsa::create(std::string name) {
  auto it = dsa_factory.find(name);
  if (it == dsa_factory.end()) {
    return nullptr;
  } else {
    return it->second();
  }
}

void dsa::register_factory(std::string name, dsa_factory_function func) {
  dsa_factory[name] = func;
}


