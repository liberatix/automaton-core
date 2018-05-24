# Automaton data schema & serialization interface

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "data",
  srcs = [
    "factory.cc",
    "schema.cc",
    "msg.cc",
  ],
  hdrs = [
    "factory.h",
    "schema.h",
    "data_exception.h",
    "msg.h",
  ],
)

cc_library(
  name = "module",
  srcs = ["module.cc"],
  hdrs = ["module.h"],
  deps = [
    ":data",
    "//automaton/core/script",
  ],
)
