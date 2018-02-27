package(default_visibility = ["//visibility:public"])

genrule(
  name = "emptycc",
  outs = ["empty.cc"],
  cmd = "echo > $@",
)

cc_library(
  name = "selene",
  srcs = ["empty.cc"],
  hdrs = glob(["**/*"]),
  includes = ["include"],
)
