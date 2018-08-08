package(default_visibility = ["//visibility:public"])

genrule(
  name = "empty_cc",
  outs = ["empty.cc"],
  cmd = "echo '#include <sol.hpp>' > $@",
)

cc_library(
  name = "sol",
  srcs = [
    "empty.cc",
  ],
  hdrs = ["sol.hpp"],
  includes = ["."],
  deps = select ({
    "//conditions:windows": ["@lua//:liblua"],
    "//conditions:default": ["@luajit//:luajit"],
  }),
)
