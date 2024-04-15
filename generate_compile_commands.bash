#!/bin/bash

source build/Release/generators/conanbuild.sh && cmake --preset conan-release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
