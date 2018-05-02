
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
    size = "large",
    deps = [
      "//crypto",
      "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "RIPEMD160_test",
    srcs = ["test_RIPEMD160_cryptopp.cc"],
    size = "small",
    deps = [
      "//crypto",
      "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "SHA512_test",
    srcs = ["test_SHA512_cryptopp.cc"],
    size = "large",
    deps = [
      "//crypto",
      "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "Keccak_256_test",
    srcs = ["test_Keccak_256_cryptopp.cc"],
    size = "small",
    deps = [
      "//crypto",
      "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "SHA3_256_test",
    srcs = ["test_SHA3_256_cryptopp.cc"],
    size = "large",
    deps = [
      "//crypto",
      "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "secp256k1_cryptopp",
    srcs = ["test_secp256k1_cryptopp.cc"],
    size = "small",
    deps = [
      "//crypto",
      "@gtest//:gtest_main",
    ],
)
