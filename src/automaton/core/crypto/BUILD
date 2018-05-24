# Automaton crypto module interface

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "crypto",
  srcs = [
    "hash_transformation.cc",
    "digital_signature.cc",
    "secure_random.cc",
  ],
  hdrs = [
    "hash_transformation.h",
    "digital_signature.h",
    "secure_random.h",
  ],
)

cc_library(
  name = "module",
  srcs = ["module.cc"],
  hdrs = ["module.h"],
  deps = [
    ":crypto",
    "//automaton/core/script",
  ],
)
