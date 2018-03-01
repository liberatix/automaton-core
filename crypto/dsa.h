#ifndef AUTOMATON_CORE_CRYPTO__TRANSFORMATION_H__
#define AUTOMATON_CORE_CRYPTO__TRANSFORMATION_H__

#include <string>
#include <map>

class dsa {
 public:
  // The size of the public key in bytes
  virtual size_t public_key_size() = 0;
  // The size of the private key in bytes
  virtual size_t private_key_size() = 0;
  // The size of hashed message in bytes
  virtual size_t hashed_message_size() = 0;
  // The size of the signature in bytes
  virtual size_t signature_size() = 0;
  // The size of the random k in bytes;
  virtual size_t k_size() = 0;

  // Returns true of this implementation can sign deterministic
  // signatures using the sign_deterministic() function
  virtual bool has_deterministic_signatures() = 0;

  // A function pointer given to the register_factory to register.
  // The function will be used by create() to instantiate a
  // derived class implementing this interface
  typedef dsa * (*dsa_factory_function)();

  // Create public key from the private. The format is implementation specific
  // Precondition public_key_size() == public_key in bytes.
  // IN:  private_key:  the private key as a buffer.
  // OUT: public_key:   a pointer to the buffer to recive the public key.
  virtual void gen_public_key(const unsigned char * private_key,
                              unsigned char * public_key) = 0;

  // Sign a message using private key and message or message hash
  // Precondition signature_size() == signature in bytes.
  // IN:  private_key:  the private key as a buffer.
  //      message:      the message as a pointer to a buffer.
  // OUT: signature:    a pointer to the buffer to recive the signature.
  virtual void sign(const unsigned char * private_key,
                    const unsigned char * message,
                    unsigned char * signature) = 0;

  // Sign a deterministic message using private key, random k
  // and message or message hash
  // Precondition signature_size() == signature in bytes.
  // IN:  private_key:  the private key as a buffer.
  //      message:      the message as a pointer to a buffer.
  //      k:            the random k used the sign the message
  // OUT: signature:    a pointer to the buffer to recive the signature.
  virtual void sign_deterministic(const unsigned char * private_key,
                                  const unsigned char * message,
                                  const unsigned char * k,
                                  unsigned char * signature) = 0;

  // Verify that a message was signed by the privated key
  // coresponding the the given public key
  // IN:  public:     the public key as a buffer.
  //      message:    the message as a pointer to a buffer.
  //      signature:  a pointer to the buffer to recive the signature.
  virtual bool verify(const unsigned char * public_key,
                      const unsigned char * message,
                      unsigned char * signature) = 0;

  // algorithm
  // secp256k1
  // secp256r1
  // ed25519
  static dsa * create(std::string algorithm);

  // Register the create function for a given implementation, will overwrite
  // already registered functions.
  // IN:  name:   a string that will be used to call this function.
  //      func:   function pointers used to instantiate classes
  //              implementing the interface.
  static void register_factory(std::string name, dsa_factory_function func);

 private:
  // Map holding the function pointers used to instantiate
  // classes implementing this interface.
  static std::map<std::string, dsa_factory_function> dsa_factory;
};

#endif  //  AUTOMATON_CORE_CRYPTO_dsa_TRANSFORMATION_H__
