# Automaton crypto library

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "crypto",
    srcs = [
        "hash_transformation.cc",
        "hash_transformation.h",
        "SHA256_cryptopp.cc",
        "SHA256_cryptopp.h",
        "RIPEMD160_cryptopp.cc",
        "RIPEMD160_cryptopp.h",
        "SHA512_cryptopp.cc",
        "SHA512_cryptopp.h",
        "Keccak_256_cryptopp.cc",
        "Keccak_256_cryptopp.h",
        "SHA3_256_cryptopp.cc",
        "SHA3_256_cryptopp.h",
        "digital_signature.cc",
        "digital_signature.h",
        "secp256k1_cryptopp.cc",
        "secp256k1_cryptopp.h",
        "secure_random.cc",
        "secure_random.h",
        "secure_random_cryptopp.cc",
        "secure_random_cryptopp.h",
        "ed25519_orlp.cc",
        "ed25519_orlp.h",
    ],
    hdrs = [
      "hash_transformation.h",
      "SHA256_cryptopp.h",
      "RIPEMD160_cryptopp.h",
      "SHA512_cryptopp.h",
      "Keccak_256_cryptopp.h",
      "SHA3_256_cryptopp.h",
      "digital_signature.h",
      "secp256k1_cryptopp.h",
      "secure_random.h",
      "secure_random_cryptopp.h",
      "ed25519_orlp.h",
      ],
    deps = [
      "@cryptopp//:cryptopp",
      "@ed25519_orlp//:ed25519_orlp",
    ],
)
