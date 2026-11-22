#!/usr/bin/env bash
# Builds and runs the engine-free core's unit tests. Needs only a C++17 compiler.
#   bash Tests/run_tests.sh
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p Tests/build
CXX="${CXX:-g++}"
"$CXX" -std=c++17 -O1 -g -Wall -Wextra -Wshadow -Werror=shadow \
	-ISource/IdealWorld/Core \
	Source/IdealWorld/Core/*.cpp Tests/core_tests.cpp \
	-o Tests/build/core_tests
./Tests/build/core_tests saves
