#ifndef AUTOMATON_CORE_CRYPTO_SIGNER_TRANSFORMATION_H__
#define AUTOMATON_CORE_CRYPTO_SIGNER_TRANSFORMATION_H__
#include <map>
#include <string>
#include "key.h"

class signer {
 public:
  signer();
  // A function pointer given to the register_factory to register.
  // The function will be used by create() to instantiate a hash_transformation
  // derived class implementing this interface
  typedef signer * (*signer_factory_function)();

  bool initialised = false;
  
  virtual std::string sign(std::string msg_hash, long long random = 0) = 0;
   
 private:
   // Map holding the function pointers used to instantiate classes implementing
   // the interface.
   static std::map<std::string, std::map<std::string, signer_factory_function> >
     pub_key_factory;
   key * private_key;
};

#endif  //  AUTOMATON_CORE_CRYPTO_SIGNER_TRANSFORMATION_H__