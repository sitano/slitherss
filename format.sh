#!/bin/sh
find . -maxdepth 3 -name '*.cc' -or -name '*.h' -exec clang-format -i {} \;
