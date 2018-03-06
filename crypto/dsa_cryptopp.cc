#include <string>
#include "crypto/dsa.h"
#include "crypto/dsa_cryptopp.h"
#include <iostream>
#include "cryptlib.h"  // NOLINT
#include "eccrypto.h"  // NOLINT
#include "integer.h"  // NOLINT
#include "oids.h"  // NOLINT
#include "randpool.h"  // NOLINT


/*
TODO(Samir): set domain params from the fallowing example
https://stackoverflow.com/a/45796422
OID curve = ASN1::secp256r1();
ECDH<ECP>::Domain domain(curve);
====================================================
// To get the size of the private and public keys:
size_t len = domain.PrivateKeyLength()
====================================================
SecByteBlock privKey(domain.PrivateKeyLength());
SecByteBlock pubKey(domain.PublicKeyLength());
*/

size_t dsa_cryptopp::public_key_size() {
  return 33;
}
size_t dsa_cryptopp::private_key_size() {
  return 32;
}
size_t dsa_cryptopp::hashed_message_size() {
  return 32;
}
size_t dsa_cryptopp::signature_size() {
  return 64;
}
// TODO(Samir): max input k size?
size_t dsa_cryptopp::k_size() {
  return 0;
}

bool dsa_cryptopp::has_deterministic_signatures() {
  return false;
}

void dsa_cryptopp::gen_public_key(const unsigned char * private_key,
                                  unsigned char * public_key) {
  // Create private key object from exponent
  const CryptoPP::Integer privateExponent(private_key, private_key_size());
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey privateKey;
  privateKey.Initialize(CryptoPP::ASN1::secp256k1(), privateExponent);
  // Create the public key object
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey publicKey;
  privateKey.MakePublicKey(publicKey);
  // Save the public key in public_key
  publicKey.GetGroupParameters().GetCurve().EncodePoint(public_key,
      publicKey.GetPublicElement(), true);
}
void dsa_cryptopp::sign(const unsigned char * private_key,
                        const unsigned char * message,
                        unsigned char * signature) {
  CryptoPP::RandomPool prng;

  std::string str_signature;
  // Create private key object from exponent
  const CryptoPP::Integer privateExponent(private_key, private_key_size());
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey privateKey;
  privateKey.Initialize(CryptoPP::ASN1::secp256k1(), privateExponent);

  memset(signature, 0, signature_size());
  CryptoPP::StringSource(message, hashed_message_size(), true,
    new CryptoPP::SignerFilter(prng, CryptoPP::ECDSA<CryptoPP::ECP,
        CryptoPP::SHA256>::Signer(privateKey),
            new CryptoPP::ArraySink(signature, signature_size())));
  // std::cout << "signature length: " << str_signature.length() << std::endl;
  // std::cout << "signature: " << str_signature << std::endl;
}
void dsa_cryptopp::sign_deterministic(const unsigned char * private_key,
                                      const unsigned char * message,
                                      const unsigned char * k,
                                      unsigned char * signature) {
  throw;
}
bool dsa_cryptopp::verify(const unsigned char * public_key,
                          const unsigned char * message,
                          unsigned char * signature) {
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey publicKey;

  std::string input(reinterpret_cast<const char*>(public_key), public_key_size());
  CryptoPP::ECP::Point p;
  publicKey.AccessGroupParameters().Initialize(CryptoPP::ASN1::secp256k1());
  publicKey.GetGroupParameters().GetCurve().DecodePoint(p, public_key,
      public_key_size());
  publicKey.SetPublicElement(p);

  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Verifier
      verifier(publicKey);
  return verifier.VerifyMessage(message, hashed_message_size(), signature,
      signature_size());
}
bool dsa_cryptopp::register_self() {dsa_cryptopp::register_factory("secp256k1",
        [] {return reinterpret_cast<dsa*>(new dsa_cryptopp()); });
  return true;
}
