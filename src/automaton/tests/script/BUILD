# Automaton scripting library tests

package(default_visibility = ["//visibility:public"])

cc_test(
  name = "test_script",
  srcs = ["test_script.cc",],
  deps = [
    "//automaton/core/crypto:module",
    "//automaton/core/crypto/cryptopp:module",
    "//automaton/core/crypto/ed25519_orlp:module",
    "//automaton/core/data:module",
    "//automaton/core/data/protobuf:module",
    "//automaton/core/io:module",
    "//automaton/core/log:module",
    "//automaton/core/network:module",
    "//automaton/core/script",
    "//automaton/core/state:module",
    "@gtest//:gtest_main",
  ],
)
