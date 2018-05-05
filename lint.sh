#!/bin/bash
function join_by { local IFS="$1"; shift; echo "$*"; }

FILTERS=(
  "-legal/copyright"
  "-build/header_guard"
  "-build/c++11"
)

FILTER_ARG=$(join_by "," "${FILTERS[@]}")

find . -type f -name BUILD -exec dirname {} \; | \
  xargs -I{} find {} -iname *.cc -o -iname *.cpp -o -iname *.h | \
  xargs cpplint.py --filter=$FILTER_ARG
