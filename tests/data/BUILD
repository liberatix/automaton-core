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
  data = ["//tests/data:test.proto"],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_invalid_data",
	srcs = ["protobuf_schema_test_invalid_data.cc",],
  data = ["//tests/data:invalid_data.proto"],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_find_existing_schema",
	srcs = ["protobuf_schema_test_find_existing_schema.cc",],
  data = ["//tests/data:test.proto"],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_find_all_fields",
	srcs = ["protobuf_schema_test_find_all_fields.cc",],
  data = ["//tests/data:many_fields.proto"],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_find_all_enums",
	srcs = ["protobuf_schema_test_find_all_enums.cc",],
  data = ["//tests/data:many_enums.proto"],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_find_enum",
	srcs = ["protobuf_schema_test_find_enum.cc",],
  data = ["//tests/data:many_enums.proto"],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_empty_schema",
	srcs = ["protobuf_schema_test_empty_schema.cc",],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_setting_fields",
	srcs = ["protobuf_schema_test_setting_fields.cc",],
  data = ["//tests/data:test.proto"],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_message_serialization",
	srcs = ["protobuf_schema_test_message_serialization.cc",],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_messages",
	srcs = ["protobuf_schema_test_messages.cc",],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)

cc_test (
	name = "protobuf_schema_test_enums",
	srcs = ["protobuf_schema_test_enums.cc",],
	deps = [
    "//data:data_google_protobufs",
    "@gtest//:gtest_main",
  ]
)
