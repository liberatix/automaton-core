# Automaton data serialization library

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "schema",
  srcs = [
    "schema.cc",
    "schema_definition.cc",
  ],
  hdrs = [
    "schema.h",
    "schema_definition.h",
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
  ],
  copts = ["-Wno-sign-compare"],
  deps = [
	  "//schema:schema",
  ],
)
