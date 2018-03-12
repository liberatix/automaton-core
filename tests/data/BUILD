cc_binary (
	name = "protobuf_schema_test",
	srcs = ["protobuf_schema_test.cc",],
	deps = ["//data:data_google_protobufs",]
)

cc_test (
	name = "protobuf_schema_definitions",
	srcs = ["protobuf_schema_gtest.cc",],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_find_unexisting_schema",
	srcs = ["protobuf_schema_test_find_unexisting_schema.cc",],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)
