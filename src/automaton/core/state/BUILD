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
    "//automaton/core/crypto",
    "//automaton/core/log",
  ],
  linkstatic=True,
)

cc_library(
  name = "state_persistent",
  srcs = [
    "state_persistent.cc",
    "state_persistent.h",
    "state.cc",
    "state.h",
  ],
  hdrs = [
    "state_persistent.h",
    "state.h",
  ],
  deps = [
    "//automaton/core/crypto",
    "//automaton/core/storage:blobstore",
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
