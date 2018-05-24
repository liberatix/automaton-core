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
)

cc_library(
  name = "module",
  srcs = ["module.cc"],
  hdrs = ["module.h"],
  deps = [
    "//automaton/core/script",
  ],
)
