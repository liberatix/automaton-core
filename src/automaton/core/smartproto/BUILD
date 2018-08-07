# Automaton Core Smart protocols

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "smartproto",
  srcs = [
    "node.cc",
  ],
  hdrs = [
    "node.h",
  ],
  deps = [
    "//automaton/core/common",
    "//automaton/core/data",
    "//automaton/core/data/protobuf",
    "//automaton/core/log",
    "//automaton/core/script",
    "//automaton/core/script/lua",
    "@json//:json",
  ],
  linkstatic=True,
  defines = select({
      "//conditions:windows": ["BOOST_ALL_NO_LIB"],
      "//conditions:default": [],
  }),
)
