#/bin/bash
cd src
mkdir -p logs
bazel build //automaton/core -c opt && bazel-bin/automaton/core/core
