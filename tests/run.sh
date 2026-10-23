#!/bin/sh
# Builds and runs the tests for the rules, without Reason or the SDK.
set -e
cd "$(dirname "$0")"
mkdir -p build
clang++ -std=c++17 -Wall -Wextra -o build/LauncherTest LauncherTest.cpp ../Launcher.cpp
clang++ -std=c++17 -Wall -Wextra -o build/PressTest PressTest.cpp ../Press.cpp ../Launcher.cpp
./build/LauncherTest
./build/PressTest
cd ..
luajit tools/check-names.lua
