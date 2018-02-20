package(default_visibility = ["//visibility:public"])

cc_library(
  name = "cryptopp",
  srcs = 
      ["cryptlib.cpp", "cpu.cpp", "integer.cpp"] +
      glob(
        ["*.cpp"],
        exclude = [
          // SRCS in GNUMakefile exclude the following.
          "cryptlib.cpp",
          "cpu.cpp",
          "integer.cpp",
          "pch.cpp",
          "simple.cpp",
          "winpipes.cpp",
          "cryptlib_bds.cpp",
          "dll.cpp",
          "adhoc.cpp",

          // Exclude tests.
          "test.cpp",
          "bench1.cpp",
          "bench2.cpp",
          "validat0.cpp",
          "validat1.cpp",
          "validat2.cpp",
          "validat3.cpp",
          "validat4.cpp",
          "datatest.cpp",
          "regtest1.cpp",
          "regtest2.cpp",
          "regtest3.cpp",
          "dlltest.cpp",
          "fipsalgt.cpp",
        ],
      ),
  hdrs = 
      glob(
        [
          "*.h",
          "algebra.cpp",
          "eprecomp.cpp",
          "polynomi.cpp",
        ],
        exclude = [
          "resource.h",
          // Exclude TESTINCL as specified in GNUMakefile
          "bench.h",
          "factory.h",
          "validate.h",
        ],
      ),
  defines = [
    "NDEBUG",
    "CRYPTOPP_DISABLE_ASM",
    "CRYPTOPP_DISABLE_AESNI",
    "CRYPTOPP_DISABLE_SHA",
    "CRYPTOPP_DISABLE_SSSE3",
    "CRYPTOPP_DISABLE_SSE4",
  ],
  copts = [
    "-g2",
    "-O3",
    "-fPIC",
    "-pthread",
    "-pipe",
    "-c",
  ],
)
