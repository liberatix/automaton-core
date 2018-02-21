package(default_visibility = ["//visibility:public"])


cc_import(
  name = "cryptopplib",
  hdrs =
      glob(
        [
          "*.h",
        ],
        exclude = [
          "resource.h",
          # Exclude TESTINCL as specified in GNUMakefile
          "bench.h",
          "factory.h",
          "validate.h",
        ],
      ),
  static_library = "cryptlib.lib", # A normal static library
)
