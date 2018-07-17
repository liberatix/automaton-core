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
      "//conditions:windows": [
        "@localboost//:iostreams",
        "@localboost//:filesystem",
      ],
      "//conditions:default": [
        "@boost//:iostreams",
        "@boost//:filesystem",
      ],
    }),
    defines = ["BOOST_ALL_NO_LIB"],
)
