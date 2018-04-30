# Automaton examples - crypto library

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "examples_crypto",
    srcs = [
        "simple_miner.cc",
    ],
    hdrs = [
      "simple_miner.h",
      ],
    deps = [
      "//crypto",
    ],
)

cc_binary(
    name = "demo_miner",
    srcs = [
      "miner_demo.cc"
    ],
    deps = [
      "//crypto",
      "examples_crypto",
    ],
)
