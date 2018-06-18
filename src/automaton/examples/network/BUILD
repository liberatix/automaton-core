# Automaton examples - first smart protocol

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "node_prototype",
    srcs = [
      "node_prototype.cc",
    ],
    hdrs = [
      "node_prototype.h",
    ],
    deps = [
      "//automaton/core/log",
      "//automaton/core/network:simulated_connection",
    ],
)
cc_binary (
	name = "simulation_big_test",
	srcs = ["simulation_big_test.cc",],
	deps = [
    "//automaton/core/network:simulated_connection",
      "//automaton/core/log",
  ],
)
cc_binary (
	name = "simulation_test_node_prototype",
	srcs = ["simulation_test_node_prototype.cc",],
	deps = [
    "//automaton/core/network:simulated_connection",
    "//automaton/examples/network:node_prototype",
    "//automaton/core/log",
  ],
)
cc_binary (
	name = "simulation_test_localhost",
	srcs = ["simulation_test_localhost.cc",],
	deps = [
    "//automaton/core/network:network_tcp",
    "//automaton/examples/network:node_prototype",
    "//automaton/core/log",
  ],
)
