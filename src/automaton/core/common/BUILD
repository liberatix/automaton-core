# Automaton Core common module

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "common",
  srcs = [
    "obj.cc",
  ],
  hdrs = [
    "obj.h",
  ],
  linkstatic=True,
)

cc_library(
  name = "module",
  srcs = ["module.cc"],
  hdrs = ["module.h"],
  data = ["module.proto"],
  deps = [
    "//automaton/core/common",
    "//automaton/core/io",
    "//automaton/core/script",
  ],
  linkstatic=True,
)
