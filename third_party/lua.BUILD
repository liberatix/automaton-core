package(default_visibility = ["//visibility:public"])

cc_library(
    name = "liblua",
    srcs = [
        # Core language
        "src/lapi.c",
        "src/lcode.c",
        "src/lctype.c",
        "src/ldebug.c",
        "src/ldo.c",
        "src/ldump.c",
        "src/lfunc.c",
        "src/lgc.c",
        "src/llex.c",
        "src/lmem.c",
        "src/lobject.c",
        "src/lopcodes.c",
        "src/lparser.c",
        "src/lstate.c",
        "src/lstring.c",
        "src/ltable.c",
        "src/ltm.c",
        "src/lundump.c",
        "src/lvm.c",
        "src/lzio.c",

        # Standard libraries
        "src/lauxlib.c",
        "src/lbaselib.c",
        "src/lbitlib.c",
        "src/lcorolib.c",
        "src/ldblib.c",
        "src/liolib.c",
        "src/lmathlib.c",
        "src/loslib.c",
        "src/lstrlib.c",
        "src/ltablib.c",
        "src/lutf8lib.c",
        "src/loadlib.c",
        "src/linit.c",
    ],
    hdrs = glob(["src/*.h"]),
    defines = ["LUA_COMPAT_5_2"],
    includes = ["src"],
)

cc_binary(
    name = "lua",
    srcs = ["src/lua.c"],
    deps = [":liblua"],
)

cc_binary(
    name = "luac",
    srcs = ["src/luac.c"],
    deps = [":liblua"],
)
