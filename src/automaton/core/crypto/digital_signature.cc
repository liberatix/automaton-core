#include "automaton/core/crypto/digital_signature.h"

namespace automaton {
namespace core {
namespace crypto {

std::map<std::string, digital_signature::digital_signature_factory_function>
    digital_signature::digital_signature_factory;

digital_signature * digital_signature::create(std::string name) {
  auto it = digital_signature_factory.find(name);
  if (it == digital_signature_factory.end()) {
    return nullptr;
  } else {
    return it->second();
  }
}

void digital_signature::register_factory(std::string name,
    digital_signature_factory_function func) {
  digital_signature_factory[name] = func;
}

common::status digital_signature::process(const obj& request, obj* response) {
  return common::status(common::OK);
}

}  // namespace crypto
}  // namespace core
}  // namespace automaton
