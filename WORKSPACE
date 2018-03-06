GTEST_LABEL = "ec44c6c1675c25b9827aacd08c02433cccde7780"
CRYPTOPP_LABEL = "6_0_0"
LUA_PREFIX = "lua-5.3.4"
SELENE_PREFIX = "Selene-0.4"

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

# boost::asio
#new_git_repository(
#  name = "com_github_boost_asio",
#  commit = "6814d260d02300a97521c1a93d02e30877fb8ff5",
#  remote = "https://github.com/boostorg/asio.git",
#  build_file = "boost_asio.BUILD",
#)

#new_http_archive(
#  name = "com_github_boost_system",
#  urls = ["https://github.com/boostorg/system/archive/boost-1.66.0.tar.gz"],
#  sha256 = "4b3a4abd23a98aa100b3ec77c455d75c003a6f34c77e4d8b6e7fd2bd30322c45",
#  build_file = "boost_system.BUILD",
#  strip_prefix = "system-boost-1.66.0",
#)

#new_local_repository(
#  name = "luajit",
#  path = "LuaJIT-2.0.5",
#  build_file = "luajit.BUILD",
#)

new_http_archive(
  name = "lua",
  url = "https://github.com/lua/lua/releases/download/v5-3-4/lua-5.3.4.tar.gz",
  sha256 = "f681aa518233bc407e23acf0f5887c884f17436f000d453b2491a9f11a52400c",
  #url = "http://www.lua.org/ftp/lua-5.3.1.tar.gz",
  #sha256 = "072767aad6cc2e62044a66e8562f51770d941e972dc1e4068ba719cd8bffac17",
  build_file = "lua.BUILD",
  strip_prefix = LUA_PREFIX,
)

new_http_archive(
  name = "selene",
  url = "https://github.com/jeremyong/Selene/archive/v0.4.tar.gz",
  sha256 = "e448981a3247541c497a7d0ca90e6a1b61f58ede5fcb4e7fc1c1ec25ed3f5a57",
  build_file = "selene.BUILD",
  strip_prefix = SELENE_PREFIX,
)

# Local repository version for selene.
# new_local_repository(
#   name = "selene",
#   path = SELENE_PREFIX,
#   build_file = "selene.BUILD",
# )

git_repository(
    name = "com_github_nelhage_rules_boost",
    commit = "239ce40e42ab0e3fe7ce84c2e9303ff8a277c41a",
    remote = "https://github.com/nelhage/rules_boost",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()
