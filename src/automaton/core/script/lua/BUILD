# Automaton Lua scripting implementation

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "lua",
  srcs = glob(["**/*.cc"]),
  hdrs = glob(["**/*.h"]),
  deps = select({
      "//conditions:windows": ["@lua//:liblua"],
      "//conditions:default": ["@luajit//:luajit"],
    }) +
    [
      "@sol//:sol",
      "//automaton/core/crypto",
      "//automaton/core/crypto/cryptopp",
      "//automaton/core/crypto/ed25519_orlp",
      "//automaton/core/data",
      "//automaton/core/io",
      "//automaton/core/script",
    ],
)
