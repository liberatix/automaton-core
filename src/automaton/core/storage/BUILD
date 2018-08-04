# Automaton storage library

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "blobstore",
    srcs = [
      "blobstore.cc",
      "blobstore.h",
    ],
    hdrs = [
      "blobstore.h",
    ],
)

cc_library(
    name = "persistent_blobstore",
    srcs = [
      "blobstore.cc",
      "blobstore.h",
      "persistent_blobstore.cc",
      "persistent_blobstore.h",
    ],
    hdrs = [
      "blobstore.h",
      "persistent_blobstore.h",
    ],
    deps = select({
      "//conditions:windows":
      [
        "@localboost//:iostreams",
        "@localboost//:filesystem",
      ],
      "//conditions:default":
      [
        "@boost//:iostreams",
        "@boost//:filesystem",
      ],
    }),
    #linkstatic = True,
    #copts = ["-MT"],
    defines = ["BOOST_ALL_NO_LIB"],
)

cc_library(
    name = "persistent_storage",
    srcs = [
      "persistent_storage.cc",
      "persistent_storage.h",
    ],
    hdrs = [
      "persistent_storage.h",
    ],
    deps = select({
      "//conditions:windows":
      [
        "@localboost//:iostreams",
        "@localboost//:filesystem",
      ],
      "//conditions:default":
      [
        "@boost//:iostreams",
        "@boost//:filesystem",
      ],
    }),
    #linkstatic = True,
    #copts = ["-MT"],
    defines = ["BOOST_ALL_NO_LIB"],
)
