#ifndef AUTOMATON_CORE_CRYPTO_PUB_KEY_TRANSFORMATION_H__
#define AUTOMATON_CORE_CRYPTO_PUB_KEY_TRANSFORMATION_H__

#include <map>
#include <string>

// Base class for generating private keys
class pub_key {
public:
  // A function pointer given to the register_factory to register.
  // The function will be used by create() to instantiate a hash_transformation
  // derived class implementing this interface
  typedef pub_key * (*pub_key_factory_function)();
  bool initialised = false;

private:
  // Map holding the function pointers used to instantiate classes implementing
  // the interface.
  static std::map<std::string, std::map<std::string, pub_key_factory_function> >
    pub_key_factory;

  std::pair<int, int> point;
  unsigned int key_lenght;
  struct params {
    std::string curve;
    std::string field;
  };

public:
  static void register_factory(std::string field, std::string curve, pub_key_factory_function func);
 
};
#endif  // AUTOMATON_CORE_CRYPTO_PUB_KEY_TRANSFORMATION_H__
