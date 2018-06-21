# Automaton scripting library

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "script",
  srcs = [
    "registry.cc",
  ],
  hdrs = [
    "registry.h",
  ],
  deps = [
    "//automaton/core/common",
    "//automaton/core/data",
    "//automaton/core/data/protobuf",
    "//automaton/core/log",
    "@boost//:regex",
  ],
  linkstatic=True,
)
