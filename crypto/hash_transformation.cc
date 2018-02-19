#include "hash_transformation.h"
using namespace std;

void hash_transformation::calculate_digest(const unsigned char* input,
                                           const size_t length,
                                           unsigned char* digest) {
  update(input, length);
  final(digest);
}

map<string, hash_transformation::factory_function_type>
            hash_transformation::hash_transformation_factory;
void hash_transformation::register_class_factory(string name,
                                                 factory_function_type func) {
  // TODO(samir): decide what to do if function is already registered
  // auto it = hash_transformation_factory.find(name);
  // if (it != hash_transformation_factory.end()) {
  // }

  hash_transformation_factory[name] = func;
}

hash_transformation* hash_transformation::create(string name) {
  auto it = hash_transformation_factory.find(name);
  if (it == hash_transformation_factory.end()) {
    return nullptr;
  }
  else {
    return it->second();
  }
}
