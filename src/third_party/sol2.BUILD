package(default_visibility = ["//visibility:public"])

genrule(
  name = "empty_cc",
  outs = ["empty.cc"],
  cmd = "echo '' > $@",
)

cc_library(
  name = "sol",
  srcs = [
    "empty.cc",
  ],
  hdrs = ["sol.hpp"],
  includes = ["."],
)
