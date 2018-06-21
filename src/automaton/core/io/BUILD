# Automaton I/O utility library

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "io",
  srcs = [
    "io.cc",
  ],
  hdrs = [
    "io.h",
  ],
  deps = [
    "//automaton/core/log",
  ],
  linkstatic=True,
)

cc_library(
  name = "module",
  srcs = ["module.cc"],
  hdrs = ["module.h"],
  data = ["module.proto"],
  deps = [
    "//automaton/core/io",
    "//automaton/core/script",
  ],
  linkstatic=True,
)
