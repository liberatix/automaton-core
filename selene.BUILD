package(default_visibility = ["//visibility:public"])

filegroup(
  name = "selene_cc_tests",
  srcs = glob(["test/**/*.h"])
)

filegroup(
  name = "selene_lua_tests",
  srcs = glob(["test/**/*.lua"])
)

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
