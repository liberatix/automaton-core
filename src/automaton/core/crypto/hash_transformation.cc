#include "automaton/core/crypto/hash_transformation.h"
#include <string>

namespace automaton {
namespace core {
namespace crypto {

void hash_transformation::calculate_digest(const uint8_t * input,
                                           const size_t length,
                                           uint8_t * digest) {
  update(input, length);
  final(digest);
}

std::map<std::string, hash_transformation::factory_function_type>
    hash_transformation::hash_transformation_factory;

void hash_transformation::register_factory(std::string name,
                                           factory_function_type func) {
  // TODO(samir): decide what to do if function is already registered
  // auto it = hash_transformation_factory.find(name);
  // if (it != hash_transformation_factory.end()) {
  // }

  hash_transformation_factory[name] = func;
}

hash_transformation * hash_transformation::create(std::string name) {
  auto it = hash_transformation_factory.find(name);
  if (it == hash_transformation_factory.end()) {
    return nullptr;
  } else {
    return it->second();
  }
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
