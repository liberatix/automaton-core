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
    ],
    hdrs = [
      "hash_transformation.h",
      "SHA256_cryptopp.h",
      "RIPEMD160_cryptopp.h",
      ],
    deps = ["@cryptopp//:cryptopp"],
)
