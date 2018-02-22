cc_library(
 name = "asio",
 visibility = ["//visibility:public"],
 includes = [
  "include/",
 ],
 hdrs = glob([
   "include/boost/**/*.h",
   "include/boost/**/*.hpp",
   "include/boost/**/*.ipp",
 ]),
 srcs = [],
 linkopts = ["-lpthread"],
 deps = ["@com_github_boost_system//:system"],
)
