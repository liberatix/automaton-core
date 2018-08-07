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
  deps = [
    "//automaton/core/script/lua",
    "@replxx//:replxx",
  ],
  linkstatic=True,
)
