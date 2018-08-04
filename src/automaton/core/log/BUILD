# Automaton logging library

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "log",
  srcs = [
    "log.cc",
    "log.h",
  ],
  hdrs = [
    "log.h",
  ],
  deps = [
    "@easyloggingpp//:easyloggingpp",
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
