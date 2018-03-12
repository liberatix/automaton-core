# Automaton data serialization library

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "data",
  srcs = [
	  "schema.cc",
  ],
  hdrs = [
	  "schema.h",
  ],
  deps = [
	  "@protobufs//protobuf-3.5.1:protobuf",
  ],
)

cc_library(
	name = "data_google_protobufs",
  srcs = [
	  "protobuf_schema.h",
	  "protobuf_schema.cc",
  ],
  copts = ["-Wno-sign-compare"],
  deps = [
	  "//data:data",
  ],
)
