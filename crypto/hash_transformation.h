#pragma once
#include <iostream>
#include <map>

using namespace std;

// Interface for hashing
class hash_transformation {
public:
  // A function pointer given to the register_class_factory to register.
  // The function will be used by create() to instantiate a hash_transformation
  // derived class implementing this interface
  typedef hash_transformation * (*factory_function_type)();

private:
  // Map holding the function pointers used to instantiate classes implementing the interface.
  static map<string, factory_function_type> hash_transformation_factory;

public:

  // Updates the hash with additional input and computes the hash of the current message.
  // Precondition digest_size == digest in bytes.
  // IN:  input:    the input as a buffer.
  //      lenght:   the size of the buffer, in bytes.
  // OUT: digest:   a pointer to the buffer to recive the hash.
  virtual void calculate_digest(const unsigned char* input, const size_t length, unsigned char* digest);

  // Update a hash with additional input.
  // IN:  input:    the additional input as a buffer.
  //      lenght:   the size of the buffer, in bytes .
  virtual void update(const unsigned char *input, const size_t length) = 0;

  // Computes the hash of the current message.
  // Precondition digest_size == digest in bytes.
  // OUT:  digest: 	a pointer to the buffer to receive the hash.
  virtual void 	final(unsigned char *digest) = 0;

  // Restarts the hash, by discarding and re-initializing the state.
  virtual void 	restart() = 0;

  // Provides the digest size of the hash.
  virtual unsigned int digest_size() const = 0;

  // Register the create function for a given implementation, will not overwrite already registered functions.
  // Returns: 1 if the function was registered, 0 if there was a function already registered with this name.
  // IN:      name:      a string name that will be used to call this function.
  //          func:      function pointers used to instantiate classes implementing the interface.
  //          (overwrite?:  optional, when true the function will be registered previously saved version for the given name with func).
  static void register_class_factory(string name, factory_function_type func);

  // Instantiate a class using the registered function in the factory.
  // Returns: Pointer to hash_transformation derived class implementing the interface or nullptr if there
  //          is no registered function with this name.
  // IN:      name:      The registered name of the function used to instantiate an implementation of this interface.
  static hash_transformation* create(const string name);

  static void print_registered_functions();
};
