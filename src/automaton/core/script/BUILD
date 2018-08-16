# Automaton scripting library

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "script",
  srcs = glob([
    "bind_*.cc",
    "engine.cc",
  ]),
  hdrs = glob(["**/*.h"]),
  deps = select({
      "//conditions:osx": ["@luajit//:luajit"],
      "//conditions:default": ["@lua//:liblua"],
    }) +
    [
      "@sol//:sol",
      "//automaton/core/crypto",
      "//automaton/core/crypto/cryptopp",
      "//automaton/core/crypto/ed25519_orlp",
      "//automaton/core/data",
      "//automaton/core/io",
    ],
    linkstatic=True,
)
