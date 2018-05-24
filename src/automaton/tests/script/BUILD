# Automaton scripting library tests

package(default_visibility = ["//visibility:public"])

cc_test(
  name = "test_script",
  srcs = ["test_script.cc",],
  deps = [
    "//automaton/core/script",
    "//automaton/core/crypto:module",
    "//automaton/core/data:module",
    "//automaton/core/io:module",
    "//automaton/core/log:module",
    "//automaton/core/network:module",
    "//automaton/core/state:module",
    "@gtest//:gtest_main",
  ],
)
