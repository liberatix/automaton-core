# Automaton Core

package(default_visibility = ["//visibility:public"])

cc_binary(
  name = "core",
  srcs = [
    "core.cc",
  ],
  deps = [
    "//automaton/core/cli",
    "//automaton/core/data",
    "//automaton/core/io",
    "//automaton/core/log",
    "//automaton/core/network",
    "//automaton/core/script",
    "//automaton/core/smartproto",
    "//automaton/core/state",
    "@json//:json",
  ],
)


cc_binary(
  name = "repl",
  srcs = [
    "cxx-api.cxx",
  ],
  deps = [
    "@replxx//:replxx"
  ],
)
