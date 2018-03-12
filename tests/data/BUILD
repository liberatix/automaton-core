cc_binary (
	name = "protobufs_test",
	srcs = ["protobufs_test.cc",],
	deps = ["//data:data_google_protobufs",]
)

cc_test (
	name = "protobuf_definitions",
	srcs = ["protobufs_gtest.cc",],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_test_find_unexisting_schema",
	srcs = ["protobuf_test_find_unexisting_schema.cc",],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)
