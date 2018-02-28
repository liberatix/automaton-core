#ifndef AUTOMATON_CORE_CRYPTO_VERYFIER_TRANSFORMATION_H__
#define AUTOMATON_CORE_CRYPTO_VERYFIER_TRANSFORMATION_H__
#include <map>
#include <string>
#include "crypto/pub_key.h"


class veryfier {
 public:
  veryfier();
  ~veryfier();
  // A function pointer given to the register_factory to register.
  // The function will be used by create() to instantiate a hash_transformation
  // derived class implementing this interface
  typedef veryfier * (*veryfier_factory_function)();
  bool initialised = false;

 private:
   // Map holding the function pointers used to instantiate classes implementing
   // the interface.
   static std::map<std::string, std::map<std::string, veryfier_factory_function> >
     pub_key_factory;
};
#endif  //  AUTOMATON_CORE_CRYPTO_VERYFIER_TRANSFORMATION_H__
