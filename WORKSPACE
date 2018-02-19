GTEST_LABEL = "ec44c6c1675c25b9827aacd08c02433cccde7780"
CRYPTOPP_LABEL = "6_0_0"

new_http_archive(
  name = "gtest",
  url = "https://github.com/google/googletest/archive/" + GTEST_LABEL + ".zip",
  sha256 = "bc258fff04a6511e7106a1575bb514a185935041b2c16affb799e0567393ec30",
  build_file = "gtest.BUILD",
  strip_prefix = "googletest-" + GTEST_LABEL,
)

http_archive(
  name = "protobufs",
  urls = ["https://github.com/google/protobuf/releases/download/v3.5.1/protobuf-cpp-3.5.1.zip"],
  sha256 = "706ac3cbd4504f34c5b991e1b4567e35d7fa883c417e1644de594a169e193af8",
)

# cryptopp-CRYPTOPP_6_0_0
new_http_archive(
  name = "cryptopp",
  url = "https://github.com/weidai11/cryptopp/archive/CRYPTOPP_6_0_0.zip",
  sha256 = "2d12dbd4ccef518124bb6065287659a5f538c4d5b4049958f6e560abb2a1bb41",
  build_file = "cryptopp.BUILD",
  strip_prefix = "cryptopp-CRYPTOPP_" + CRYPTOPP_LABEL,
)
