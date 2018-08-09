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
    "//automaton/core/network",
    "//automaton/core/network:network_tcp",
    "//automaton/core/script",
    "//automaton/core/smartproto",
    "//automaton/core/state",
    "@json//:json",
  ],
)
