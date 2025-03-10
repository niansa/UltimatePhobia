#! /bin/bash

find UltimatePhobia -iname *.hpp -o -iname *.cpp | grep -v build | grep -v generated | xargs clang-format-19 -i -style=file:.clang-format --verbose
