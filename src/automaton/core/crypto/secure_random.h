#ifndef AUTOMATON_CORE_CRYPTO_SECURE_RANDOM_H__
#define AUTOMATON_CORE_CRYPTO_SECURE_RANDOM_H__

#include <stdint.h>
#include <map>
#include <string>

#include "automaton/core/common/obj.h"

namespace automaton {
namespace core {
namespace crypto {

// Class used for getting cryptographically secure random
class secure_random : public common::obj {
 public:
  /**
    Handles process requests from script and routing to corresponding method.
  */
  common::status process(const obj& request, obj** response);

  // Generate random bit
  virtual bool bit() = 0;

  // Generate random array of bytes.
  // Instantiate a class using the registered function in the factory.
  // IN:  size:       The size of the memblock block in bytes.
  // OUT: memblock:   Memory location to save teh random bytes.
  virtual void block(uint8_t * memblock, size_t size) = 0;

  // Generate random array of byte
  virtual uint8_t byte() = 0;


  // A function pointer given to register_factory.
  // The function will be used by create() to instantiate a secure_random
  // derived class implementing this interface
  typedef secure_random * (*secure_random_factory_function)();

  // Instantiate a class using the registered function in the factory.
  // Returns:    Pointer to secure_random derived class implementing the
  //             interface or nullptr if there is no registered function
  //             with this name.
  // IN:  name:  The registered name of the function used to instantiate
  //             an implementation of this interface.
  static secure_random * create(std::string name);

  // Register the create function for a given implementation, will overwrite
  // already registered functions.
  // IN:  name:   a string that will be used to call this function.
  //      func:   function pointers used to instantiate classes
  //              implementing the interface.
  static void register_factory(std::string name,
      secure_random_factory_function func);

 private:
  // Map holding the function pointers used to instantiate classes implementing
  // the interface.
  static std::map<std::string, secure_random_factory_function>
     secure_random_factory;
};

}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  // AUTOMATON_CORE_CRYPTO_SECURE_RANDOM_H__
