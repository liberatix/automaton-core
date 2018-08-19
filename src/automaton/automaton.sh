#/bin/bash
cd automaton
mkdir -p logs
bazel build //automaton/core -c opt && ../bazel-bin/automaton/core/core
