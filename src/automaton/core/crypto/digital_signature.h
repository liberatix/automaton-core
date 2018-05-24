#ifndef AUTOMATON_CORE_CRYPTO_DIGITAL_SIGNATURE_H__
#define AUTOMATON_CORE_CRYPTO_DIGITAL_SIGNATURE_H__

#include <string>
#include <map>

namespace automaton {
namespace core {
namespace crypto {

/**
  Digital signature interface
*/
class digital_signature {
 public:
  /** The size of the public key in bytes */
  virtual size_t public_key_size() = 0;

  /** The size of the private key in bytes */
  virtual size_t private_key_size() = 0;

  /** The size of the signature in bytes */
  virtual size_t signature_size() = 0;

  /** The size of the random k in bytes; */
  virtual size_t k_size() = 0;

  /**
    A function pointer given to the register_factory to register.
    The function will be used by create() to instantiate a derived class
    implementing this interface
  */
  typedef digital_signature * (*digital_signature_factory_function)();

  /**
    Returns true of this implementation can sign deterministic
    signatures using the sign_deterministic() function
  */
  virtual bool has_deterministic_signatures() = 0;

  /**
    Generates a public key from the given private key.

    The format is implementation specific.
    
    @param[in]  private_key the private key as a buffer.
    @param[out] public_key  a pointer to the buffer to recive the public key.

    @pre public_key_size() == public_key in bytes.
    @pre private_key_size() == private_key in bytes.
  */
  virtual void gen_public_key(const unsigned char * private_key,
                              unsigned char * public_key) = 0;

  /**
    Signs a message using a private key and a message.

    @param[in]  private_key the private key as a buffer.
    @param[in]  message     the message as a pointer to a buffer.
    @param[in]  msg_len     the lenght of the message that will be signed.
    @param[out] signature   a pointer to the buffer to receive the signature.

    @pre Precondition signature_size() == signature in bytes.
  */
  virtual void sign(const unsigned char * private_key,
                    const unsigned char * message,
                    const size_t msg_len,
                    unsigned char * signature) = 0;

  /**
    Signs a deterministic message using a private key, random k and message.

    @param[in]  private_key the private key as a buffer
    @param[in]  message     the message as a pointer to a buffer
    @param[in]  msg_len     the lenght of the message that will be signed
    @param[in]  k           the random k used the sign the message
    @param[out] signature   a pointer to the buffer to recive the signature

    @pre Precondition signature_size() == signature in bytes.
  */
  virtual void sign_deterministic(const unsigned char * private_key,
                                  const unsigned char * message,
                                  const size_t msg_len,
                                  const unsigned char * k,
                                  unsigned char * signature) = 0;

  /**
    Verifies that a message was signed by the privated key coresponding to
    the given public key.

    @param[in]  public_key  the public key as a buffer
    @param[in]  message     the message as a pointer to a buffer
    @param[in]  msg_len     the lenght of the message
    @param[in]  signature   a pointer to the buffer to recive the signature
  */
  virtual bool verify(const unsigned char * public_key,
                      const unsigned char * message,
                      const size_t msg_len,
                      unsigned char * signature) = 0;

  /**
    Instantiates a class using the registered function in the factory.

    @returns Pointer to digital_signature derived class implementing the
        interface or nullptr if there is no registered function with this name.

    @param[in] name The registered name of the function used to
        instantiate an implementation of this interface.
  */
  static digital_signature * create(std::string name);

  /**
    Registers the create function for a given implementation, will overwrite
    already registered functions.

    @param[in] name a string that will be used to call this function.
    @param[in] func function pointers used to instantiate classes
        implementing the interface.
  */
  static void register_factory(std::string name,
                               digital_signature_factory_function func);

 private:
  /**
    Map holding the function pointers used to instantiate classes implementing
    this interface.
  */
  static std::map<std::string,
      digital_signature_factory_function> digital_signature_factory;
};

}  // namespace crypto
}  // namespace core
}  // namespace automaton

#endif  //  AUTOMATON_CORE_CRYPTO_DIGITAL_SIGNATURE_H__
