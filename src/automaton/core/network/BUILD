# Automaton network library

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "network",
  srcs = [
    "acceptor.cc",
    "connection.cc",
  ],
  hdrs = [
    "acceptor.h",
    "connection.h",
  ],
  deps = [
    "@boost//:asio",
  ],
)

cc_library(
  name = "network_tcp",
  srcs = [
    "tcp_implementation.h",
    "tcp_implementation.cc",
  ],
  deps = [
    ":network",
    "//automaton/core/log",
  ],
)

cc_library(
  name = "simulated_connection",
  srcs = [
    "simulated_connection.h",
    "simulated_connection.cc",
  ],
  deps = [
    ":network",
    "//automaton/core/log",
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