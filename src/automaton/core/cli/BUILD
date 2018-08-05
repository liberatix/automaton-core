# Automaton Core command line interface

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "cli",
  srcs = [
    "cli.cc",
  ],
  hdrs = [
    "cli.h",
  ],
  linkstatic=True,
)

cc_library(
  name = "module",
  srcs = ["module.cc"],
  hdrs = ["module.h"],
  data = ["module.proto"],
  deps = [
    "//automaton/core/data/protobuf",
    "//automaton/core/io",
    "//automaton/core/script",
  ],
  linkstatic=True,
)
