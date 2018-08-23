#/bin/bash
cd src
mkdir -p logs
bazel build -c opt //automaton/core && bazel-bin/automaton/core/core
