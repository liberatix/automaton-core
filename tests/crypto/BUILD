
cc_test(
    name = "test_crypto",
    srcs = ["test_hash_transformation.cc"],
    size = "small",
    deps = [
      "//crypto",
      "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "sha_test",
    srcs = ["test_SHA256_cryptopp.cc"],
    size = "small",
    deps = [
      "//crypto",
      "@gtest//:gtest_main",
    ],
)
