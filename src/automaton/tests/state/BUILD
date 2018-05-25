
cc_test(
    name = "state",
    srcs = ["state_test.cc"],
    size = "medium",
    timeout = "moderate",
    deps = [
      "//automaton/core/crypto",
      "//automaton/core/crypto/cryptopp",
      "//automaton/core/state",
      "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "state_persistent",
    srcs = ["state_persistent_test.cc"],
    size = "medium",
    timeout = "moderate",
    deps = [
      "//crypto",
      "//crypto/cryptopp",
      "//state:state_persistent",
      "@gtest//:gtest_main",
    ],
)
