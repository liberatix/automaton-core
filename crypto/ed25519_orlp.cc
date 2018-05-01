#include "crypto/ed25519_orlp.h"
#include <cryptlib.h>
#include <string>
#include <iostream>
#include "ed25519.h" // NOLINT

// unsigned char seed[32];
// unsigned char signature[64];
// unsigned char public_key[32];
// unsigned char private_key[64];

size_t ed25519_orlp::public_key_size() {
  return 32;
}
// This is actually the seed
size_t ed25519_orlp::private_key_size() {
  return 32;
}
size_t ed25519_orlp::hashed_message_size() {
  return 32;
}
size_t ed25519_orlp::signature_size() {
  return 64;
}
// TODO(Samir): max input k size?
size_t ed25519_orlp::k_size() {
  return 0;
}

bool ed25519_orlp::has_deterministic_signatures() {
  return true;
}

void ed25519_orlp::gen_public_key(const unsigned char * private_key,
                                      unsigned char * public_key) {
  unsigned char private_from_seed[64];
  ed25519_create_keypair(public_key, private_from_seed, private_key);
}

void ed25519_orlp::sign(const unsigned char * private_key,
                            const unsigned char * message,
                            unsigned char * signature) {
}

void ed25519_orlp::sign_deterministic(const unsigned char * private_key,
                                          const unsigned char * message,
                                          const unsigned char * k,
                                          unsigned char * signature) {
  sign(private_key, message, signature);
}

bool ed25519_orlp::verify(const unsigned char * public_key,
                              const unsigned char * message,
                              unsigned char * signature) {
  return 0;
}

bool ed25519_orlp::register_self() {
    ed25519_orlp::register_factory("ed25519_orlp",
    [] {return reinterpret_cast<digital_signature*>(new ed25519_orlp()); });
return true;
}
