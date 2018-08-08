# Automaton examples - Lua integration

package(default_visibility = ["//visibility:public"])

cc_binary(
  name = "demo_lua",
  srcs = [
    "demo_lua.cc",
  ],
  deps = [
    "@replxx//:replxx",
    "//automaton/core/crypto:module",
    "//automaton/core/crypto",
    "//automaton/core/crypto/cryptopp:module",
    "//automaton/core/crypto/ed25519_orlp:module",
    "//automaton/core/data:module",
    "//automaton/core/data/protobuf:module",
    "//automaton/core/io",
    "//automaton/core/network:module",
    "//automaton/core/script",
    "//automaton/core/script/lua",
    "//automaton/core/state:module",
  ],
  data = [
    "benchmark.lua",
    "blockchain.lua",
    "blockchain.proto",
    "data.lua",
    "script_demo.lua",
  ],
)
