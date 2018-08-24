#/bin/bash
cd src
mkdir -p logs
bazel build //automaton/core && bazel-bin/automaton/core/core 2>/dev/null
reset

