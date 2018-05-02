#include "crypto/secure_random.h"

std::map<std::string, secure_random::secure_random_factory_function>
    secure_random::secure_random_factory;

secure_random * secure_random::create(std::string name) {
  auto it = secure_random_factory.find(name);
  if (it == secure_random_factory.end()) {
    return nullptr;
  } else {
    return it->second();
  }
}

void secure_random::register_factory(std::string name,
    secure_random_factory_function func) {
  secure_random_factory[name] = func;
}
