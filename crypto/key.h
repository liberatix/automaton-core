#ifndef AUTOMATON_CORE_CRYPTO_KEY_TRANSFORMATION_H__
#define AUTOMATON_CORE_CRYPTO_KEY_TRANSFORMATION_H__

#include <map>
#include <string>

// Base class for generating private keys
class key {
 public:
  // A function pointer given to the register_factory to register.
  // The function will be used by create() to instantiate a hash_transformation
  // derived class implementing this interface
  typedef key * (*key_factory_function)();
  bool initialised = false;

 private:
  // Map holding the function pointers used to instantiate classes implementing
  // the interface.
  static std::map<std::string, std::map<std::string, key_factory_function> >
      key_factory;
  unsigned char * private_exponent;
  unsigned int key_lenght;
  struct params {
    std::string curve;
    std::string field;
  };

 public:
  static void register_factory(std::string field, std::string curve, 
      key_factory_function func);
  unsigned int get_key_lenght() const;
  const unsigned char * get_private_exponent();
  key::params get_params() const;
  virtual void generate_random_key() = 0;
};
#endif  // AUTOMATON_CORE_CRYPTO_KEY_TRANSFORMATION_H__

// REGISTERKEYFACTORY(ECD, CURVE);
// registerfactory<ecd, curve> somename?;
