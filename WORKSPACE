# LOCAL REPOSITORIES

new_local_repository(
  name = "ed25519_orlp",
  path = "third_party/ed25519-7fa6712ef5d581a6981ec2b08ee623314cd1d1c4/src",
  build_file = "third_party/ed25519_orlp.BUILD",
)

new_local_repository(
  name = "gtest",
  path = "third_party/googletest-ec44c6c1675c25b9827aacd08c02433cccde7780",
  build_file = "third_party/gtest.BUILD",
)

local_repository(
  name = "com_google_protobuf",
  path = "third_party/protobuf-3.5.1",
)

new_local_repository(
  name = "cryptopp",
  path = "third_party/cryptopp-CRYPTOPP_7_0_0",
  build_file = "third_party/cryptopp.BUILD",
)

new_local_repository(
  name = "easyloggingpp",
  path = "third_party/easyloggingpp-9.96.4/src",
  build_file = "third_party/easyloggingpp.BUILD",
)

new_local_repository(
  name = "bitcoin",
  path = "third_party/bitcoin-0.16.0",
  build_file = "third_party/bitcoin.BUILD",
)

new_local_repository(
  name = "lua",
  path = "third_party/lua-5.3.4",
  build_file = "third_party/lua.BUILD",
)

new_local_repository(
  name = "selene",
  path = "third_party/Selene-0.4",
  build_file = "third_party/selene.BUILD",
)

new_local_repository(
  name   = "com_github_glog_glog",
  path = "third_party/glog-0.3.5",
  build_file = "third_party/glog.BUILD",
)

# new_local_repository(
#   name = "nlohmann_json",
#   path = "third_party/json-3.1.2",
#   build_file = "json.BUILD",
# )

# Local Bazel ready builds

local_repository(
  name = "com_github_gflags_gflags",
  path = "third_party/gflags-2.2.1",
)

#new_local_repository(
#  name = "luajit",
#  path = "LuaJIT-2.0.5",
#  build_file = "luajit.BUILD",
#)

# REMOTE REPOSITORIES

# BOOST
git_repository(
  name = "com_github_nelhage_rules_boost",
  commit = "239ce40e42ab0e3fe7ce84c2e9303ff8a277c41a",
  remote = "https://github.com/nelhage/rules_boost",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

# GTEST_LABEL = "ec44c6c1675c25b9827aacd08c02433cccde7780"
# CRYPTOPP_LABEL = "7_0_0"
# LUA_PREFIX = "lua-5.3.4"
# SELENE_PREFIX = "Selene-0.4"

# Import the gflags files.
# git_repository(
#     name   = "com_github_gflags_gflags",
#     commit = "f8a0efe03aa69b3336d8e228b37d4ccb17324b88",
#     remote = "https://github.com/gflags/gflags.git",
# )

# Import the glog files.
# new_git_repository(
#     name   = "com_github_glog_glog",
#     build_file = "glog.BUILD",
#     remote = "https://github.com/google/glog.git",
#     tag = "v0.3.5",
# )

# new_http_archive(
#   name = "gtest",
#   url = "https://github.com/google/googletest/archive/" + GTEST_LABEL + ".zip",
#   sha256 = "bc258fff04a6511e7106a1575bb514a185935041b2c16affb799e0567393ec30",
#   build_file = "gtest.BUILD",
#   strip_prefix = "googletest-" + GTEST_LABEL,
# )

# http_archive(
#   name = "protobufs",
#   urls = ["https://github.com/google/protobuf/releases/download/v3.5.1/protobuf-cpp-3.5.1.zip"],
#   sha256 = "706ac3cbd4504f34c5b991e1b4567e35d7fa883c417e1644de594a169e193af8",
# )

# cryptopp-CRYPTOPP_7_0_0
# new_http_archive(
#   name = "cryptopp",
#   url = "https://github.com/weidai11/cryptopp/archive/CRYPTOPP_7_0_0.zip",
#   sha256 = "d63659f7ffd7c928bb1b67eca1c8b2f6ec743b14688f257890b1549013075d02",
#   build_file = "cryptopp.BUILD",
#   strip_prefix = "cryptopp-CRYPTOPP_" + CRYPTOPP_LABEL,
# )

# new_http_archive(
#   name = "bitcoin",
#   url = "https://github.com/bitcoin/bitcoin/archive/v0.16.0.zip",
#   sha256 = "5c1743f91b25acca53ea147a9aee9754e9489abf187d5806c0f0b8bfc11b8fbf",
#   build_file = "bitcoin.BUILD",
#   strip_prefix = "bitcoin-0.16.0",
# )

# new_http_archive(
#   name = "lua",
#   url = "https://github.com/lua/lua/releases/download/v5-3-4/lua-5.3.4.tar.gz",
#   sha256 = "f681aa518233bc407e23acf0f5887c884f17436f000d453b2491a9f11a52400c",
#   #url = "http://www.lua.org/ftp/lua-5.3.1.tar.gz",
#   #sha256 = "072767aad6cc2e62044a66e8562f51770d941e972dc1e4068ba719cd8bffac17",
#   build_file = "lua.BUILD",
#   strip_prefix = LUA_PREFIX,
# )

# new_http_archive(
#   name = "selene",
#   url = "https://github.com/jeremyong/Selene/archive/v0.4.tar.gz",
#   sha256 = "e448981a3247541c497a7d0ca90e6a1b61f58ede5fcb4e7fc1c1ec25ed3f5a57",
#   build_file = "selene.BUILD",
#   strip_prefix = SELENE_PREFIX,
# )
