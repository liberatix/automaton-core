#include "key.h"

std::map<std::string, std::map<std::string, key::key_factory_function> >
key::key_factory;

void key::register_factory(std::string field, std::string curve, key_factory_function func) {
}

unsigned int key::get_key_lenght() const {
  return key_lenght;
}

const unsigned char * key::get_private_exponent() {
  return private_exponent;
}

key::params key::get_params() const {
  return key::params();
}
