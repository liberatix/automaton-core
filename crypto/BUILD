# Automaton crypto library

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "crypto",
    linkstatic = 1,
    srcs = [
        "hash_transformation.cc",
        "hash_transformation.h",
        "SHA256_cryptopp.cc",
        "SHA256_cryptopp.h",
    ],
    hdrs = [
      "hash_transformation.h",
      "SHA256_cryptopp.h",
      ],
    #deps = ["@cryptopp//:cryptopp"],
    deps = ["//crypto/cryptopp:cryptopplib"]
)
