
package(default_visibility = ["//visibility:public"])

cc_library (
  name = "config",
  srcs = glob([
    "config.hpp"
  ]),
  hdrs = glob([
    "boost/**/*.hpp"
  ]),
  includes = ["."],
  linkstatic = True,
  defines = ["BOOST_ALL_NO_LIB"],
)

cc_library (
  name = "system",
  srcs = glob([
    "libboost_system-vc141-mt-s-x64-1_67.lib"
  ]),
  hdrs = glob([
    "boost/**/*.hpp"
  ]),
  includes = ["."],
  deps = [
    ":config",
  ],
  linkstatic = True,
  defines = ["BOOST_ALL_NO_LIB"],
)

cc_library (
  name = "filesystem",
  srcs = glob([
    "libboost_filesystem-vc141-mt-s-x64-1_67.lib"
  ]),
  hdrs = glob([
    "boost/**/*.hpp"
  ]),
  includes = ["."],
  deps = [
    ":config",
    ":system",
  ],
  linkstatic = True,
  defines = ["BOOST_ALL_NO_LIB"],
)

cc_library (
  name = "iostreams",
  srcs = [
    "libboost_iostreams-vc141-mt-s-x64-1_67.lib",
  ],
  hdrs = glob([
    "boost/**/*.hpp",
  ]),
  includes = ["."],
  deps = [
    ":config",
  ],
  linkstatic = True,
  defines = ["BOOST_ALL_NO_LIB"],
)
