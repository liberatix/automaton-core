package(default_visibility = ["//visibility:public"])

cc_library(
  name = "luajit",
  srcs = select({
    "//conditions:windows": ["luajit.lib"],
    "//conditions:default": ["libluajit.a"],
  }),
  hdrs = glob(["*.h", "*.hpp"]),
  includes = ["."],
  linkopts = select({
    "//conditions:osx": [
      "-pagezero_size 10000",
      "-image_base 100000000",
    ],
    "//conditions:default": [
    
    ],
  }),
)
