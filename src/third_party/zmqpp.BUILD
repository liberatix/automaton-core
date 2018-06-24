package(default_visibility = ["//visibility:public"])

# bazel build --copt="-DZMQ_STATIC" @zmqpp//:zmqpp
cc_library(
  name = "zmqpp",
  srcs = [
    "actor.cpp",
    "auth.cpp",
    "context.cpp",
    "curve.cpp",
    "frame.cpp",
    "message.cpp",
    "loop.cpp",
    "poller.cpp",
    "proxy_steerable.cpp",
    "proxy.cpp",
    "reactor.cpp",
    "signal.cpp",
    "socket.cpp",
    "z85.cpp",
    "zap_request.cpp",
    "zmqpp.cpp",
  ],
  hdrs = [
    "actor.hpp",
    "auth.hpp",
    "byte_ordering.hpp",
    "compatibility.hpp",
    "context_options.hpp",
    "context.hpp",
    "curve.hpp",
    "exception.hpp",
    "frame.hpp",
    "inet.hpp",
    "loop.hpp",
    "message.hpp",
    "poller.hpp",
    "proxy_steerable.hpp",
    "proxy.hpp",
    "reactor.hpp",
    "signal.hpp",
    "socket_mechanisms.hpp",
    "socket_options.hpp",
    "socket_types.hpp",
    "socket.hpp",
    "z85.hpp",
    "zap_request.hpp",
    "zmqpp.hpp",
    "external/zmqpp/zmqpp_export.h",
    "external/zmqpp/zmq_utils.h",
  ],
  deps = [
    ":zmq"
  ],
  linkstatic = 1,
  linkopts = ["-DEFAULTLIB:external/lib/libzmq.lib"]
)

cc_import(
  name = "zmq",
  hdrs = [
    "external/zmqpp/zmq.h"
  ],
  static_library = "external/lib/libzmq.lib",
)