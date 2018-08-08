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
    "//automaton/core/io",
    "@boost//:regex",
  ],
  defines = select({
      "//conditions:windows": ["BOOST_ALL_NO_LIB"],
      "//conditions:default": [],
  }),
)
