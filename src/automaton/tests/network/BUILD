cc_binary (
	name = "tcp_test",
	srcs = ["tcp_test.cc",],
	deps = [
    "//automaton/core/io",
    "//automaton/core/network:network_tcp",
  ],
)

cc_binary(
  name = "sim_test",
  srcs = [
    "sim_test.cc",
  ],
  deps = [
    "//automaton/core/io",
    "//automaton/core/network",
    "//automaton/core/network:simulated_connection",
  ],
)
