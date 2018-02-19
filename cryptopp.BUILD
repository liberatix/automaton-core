
cc_library(
  name = "cryptopp",
  srcs = 
      ["cryptlib.cpp", "cpu.cpp", "integer.cpp"] +
      glob(
        ["*.cpp"],
        exclude = [
          "cryptlib.cpp",
          "cpu.cpp",
          "integer.cpp",
          "pch.cpp",
          "simple.cpp",
          "winpipes.cpp",
          "cryptlib_bds.cpp",
          "dll.cpp",
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
        exclude = ["resource.h"],
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
