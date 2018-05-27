# Automaton examples - first smart protocol

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "first_sp",
    srcs = [
      "first_sp.cc",
    ],
    hdrs = [
      "first_sp.h",
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
    "//automaton/core/log",
    "//automaton/core/network:simulated_connection",
  ],
)
cc_binary (
	name = "simulation_test_first_sp",
	srcs = ["simulation_test_first_sp.cc",],
	deps = [
    "//automaton/core/log",
    "//automaton/core/network:simulated_connection",
    "//automaton/examples/network:first_sp",
  ],
)
