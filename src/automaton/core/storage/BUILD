# Automaton storage library

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "storage",
    srcs = [
      "blobstore.cc",
      "blobstore.h",
    ],
    hdrs = [
      "blobstore.h",
    ],
    deps = select({
      "//conditions:windows": ["@localboost//:iostreams"],
      "//conditions:default": ["@boost//:iostreams"],
    }),
    defines = ["BOOST_ALL_NO_LIB"],
)
