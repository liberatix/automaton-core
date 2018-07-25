# Bazel build definitions for locally built cppzmq

package(default_visibility = ["//visibility:public"])

genrule(
  name = "empty_cc",
  outs = ["empty.cc"],
  cmd = "echo '' > $@",
)

cc_library (
  name = "cppzmq",
  srcs = ["empty.cc"],
  hdrs = glob([
    "**/*.h",
    "**/*.hpp",
  ]),
  includes = ["."],
  deps = [
    "@libzmq//:libzmq",
  ],
  linkstatic = True,
)
