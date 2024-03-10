#!/usr/bin/bash

#Clang Format
find src -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i

# Clang Tidy
find src -iname '*.hpp' -o -iname '*.cpp' | xargs clang-tidy -p build
