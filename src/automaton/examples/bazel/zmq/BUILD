# bazel build --action_env=USE_MSVC_WRAPPER=1 //automaton/examples/bazel/zmq:zmq_demo

# known problems under windows - fails to link - fatal error LNK1104: cannot open file 'external/lib/libzmq.lib'

# Automaton examples - ZMQ
#
# package(default_visibility = ["//visibility:public"])
# 
# cc_binary (
# 	name = "zmq_demo",
# 	srcs = [
#     "zmq_server.cc",
#     "zmqpp.hpp",
#   ],
# 	deps = [
#     "@zmqpp//:zmqpp",
#   ],
#   linkopts = ["-DEFAULTLIB:external/lib/libzmq.lib"]
# )