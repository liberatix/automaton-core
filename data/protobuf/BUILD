# Automaton data schema & serialization implementation using google::protobuf

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "protobuf",
  srcs = [
    "protobuf_factory.cc",
    "protobuf_factory.h",
    "protobuf_schema.cc",
    "protobuf_schema.h",
    "protobuf_msg.cc",
    "protobuf_msg.h",
  ],
  deps = [
    "//data",
    "//log",
	  "@com_google_protobuf//:protobuf",
  ],
)
