# Automaton data schema & serialization library

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "data",
  srcs = [
    "factory.cc",
    "schema_definition.cc",
    "schema_message.cc",
  ],
  hdrs = [
    "factory.h",
    "schema_definition.h",
    "schema_exception.h",
    "schema_message.h",
  ],
  deps = [
	  "@com_google_protobuf//:protobuf",
  ],
)

cc_library(
	name = "schema_google_protobufs",
  srcs = [
    "protobuf_schema.cc",
	  "protobuf_schema.h",
    "protobuf_schema_definition.cc",
    "protobuf_schema_definition.h",
    "protobuf_schema_message.cc",
    "protobuf_schema_message.h",
  ],
  deps = [
	  "//data",
  ],
)
