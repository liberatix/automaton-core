# Automaton crypto implementation using orlp/ed25519

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "ed25519_orlp",
  srcs = [
    "ed25519_orlp.cc",
  ],
  hdrs = [
    "ed25519_orlp.h",
  ],
  deps = [
    "//automaton/core/crypto",
    "@ed25519_orlp//:ed25519_orlp",
  ],
  linkstatic=True,
)

cc_library(
  name = "module",
  srcs = ["module.cc"],
  hdrs = ["module.h"],
  data = ["module.proto"],
  deps = [
    "//automaton/core/crypto/ed25519_orlp",
    "//automaton/core/data/protobuf",
    "//automaton/core/io",
    "//automaton/core/script",
  ],
  linkstatic=True,
)
