# Automaton examples - first smart protocol

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "extended_node_prototype",
    srcs = [
      "extended_node_prototype.cc",
    ],
    hdrs = [
      "extended_node_prototype.h",
    ],
    data = ["//automaton/examples/network/extended_node:sp.proto"],
    deps = [
      "//automaton/core/io",
      "//automaton/core/log",
      "//automaton/core/network:network",
      "//automaton/core/data/protobuf",
    ],
)

cc_binary (
	name = "simulation_test_extended_node_prototype",
	srcs = ["simulation_test_extended_node_prototype.cc",],
	deps = [
    "//automaton/core/log",
    "//automaton/core/network:simulated_connection",
    "//automaton/examples/network/extended_node:extended_node_prototype",
  ],
)
