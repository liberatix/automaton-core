# Automaton Lua scripting implementation

package(default_visibility = ["//visibility:public"])

cc_library(
  name = "lua",
  srcs = glob(["**/*.cc"]),
  hdrs = glob(["**/*.h"]),
  deps = [
    "//automaton/core/data",
    "//automaton/core/script",
    "@lua//:liblua",
  ],
)
