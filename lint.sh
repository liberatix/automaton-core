#!/bin/bash
find . -path ./third_party -prune -o -path "./bazel-*" -prune -o -iname *.cc -o -iname *.cpp -o -iname *.h | xargs cpplint.py --filter=-legal/copyright,-build/header_guard,-build/c++11,-build/include_order
