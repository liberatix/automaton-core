# Automaton state library

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "state",
    srcs = [
      "state_impl.cc",
      "state_impl.h",
      "state.cc",
      "state.h",
    ],
    hdrs = [
      "state_impl.h",
      "state.h",
    ],
    deps = [
      "//crypto",
    ],
)

cc_library(
  name = "module",
  srcs = ["module.cc"],
  hdrs = ["module.h"],
  deps = [
    "//script",
  ],
)
