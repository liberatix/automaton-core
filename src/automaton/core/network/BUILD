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
  linkstatic=True,
)

cc_library(
  name = "network_tcp",
  srcs = [
    "tcp_implementation.h",
    "tcp_implementation.cc",
  ],
  deps = [
    "@boost//:asio",
    ":network",
    "//automaton/core/io",
  ],
  linkstatic=True,
  defines = select({
      "//conditions:windows": ["BOOST_ALL_NO_LIB"],
      "//conditions:default": [],
  }),

)

cc_library(
  name = "simulated_connection",
  srcs = [
    "simulated_connection.h",
    "simulated_connection.cc",
  ],
  deps = [
    ":network",
    "//automaton/core/io",
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
