cc_binary (
	name = "protobuf_schema_test",
	srcs = ["protobuf_schema_test.cc",],
	deps = ["//schema:schema_google_protobufs"],
)

cc_test (
	name = "protobuf_schema_definitions",
	srcs = ["protobuf_schema_gtest.cc",],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_find_unexisting_schema",
	srcs = ["protobuf_schema_test_find_unexisting_schema.cc",],
  data = ["//tests/schema:test.proto"],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_invalid_data",
	srcs = ["protobuf_schema_test_invalid_data.cc",],
  data = ["//tests/schema:invalid_data.proto"],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_find_existing_schema",
	srcs = ["protobuf_schema_test_find_existing_schema.cc",],
  data = ["//tests/schema:test.proto"],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_find_all_fields",
	srcs = ["protobuf_schema_test_find_all_fields.cc",],
  data = ["//tests/schema:many_fields.proto"],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_find_all_enums",
	srcs = ["protobuf_schema_test_find_all_enums.cc",],
  data = ["//tests/schema:many_enums.proto"],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_find_enum",
	srcs = ["protobuf_schema_test_find_enum.cc",],
  data = ["//tests/schema:many_enums.proto"],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_empty_schema",
	srcs = ["protobuf_schema_test_empty_schema.cc",],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_setting_fields",
	srcs = ["protobuf_schema_test_setting_fields.cc",],
  data = ["//tests/schema:test.proto"],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_message_serialization",
	srcs = ["protobuf_schema_test_message_serialization.cc",],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_messages",
	srcs = ["protobuf_schema_test_messages.cc",],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)

cc_test (
	name = "protobuf_schema_test_enums",
	srcs = ["protobuf_schema_test_enums.cc",],
	deps = [
    "//schema:schema_google_protobufs",
    "@gtest//:gtest_main",
  ],
  size = "small",
)
