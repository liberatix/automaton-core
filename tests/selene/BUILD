# Adapted Selene's test main cc file, so it finds the data it needs.

cc_test(
  name = "selene_test",
  srcs = [
    "test_selene.cc",
    "@selene//:selene_cc_tests",
  ],
  size = "small",
  data = ["@selene//:selene_lua_tests"],
  deps = [
      "@lua//:liblua",
      "@selene//:selene",
  ]
)
