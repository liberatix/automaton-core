#include "crypto/pub_key.h"

std::map<std::string, std::map<std::string,
    pub_key::pub_key_factory_function> > pub_key::pub_key_factory;

void pub_key::register_factory(std::string field,
    std::string curve, pub_key_factory_function func) {
  pub_key_factory[field][curve] = func;
}
