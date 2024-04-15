# grav
N-body gravity simulator using C++, EnTT, SFML, CMake, Quill, Eigen, and Conan

Requirements
 - [Conan 2](https://docs.conan.io/2/installation.html), then use the following commands
 - [CMake](https://cmake.org/download/)

If you are using vscode, you can copy the `.vscode_recommended` folder to `.vscode` and use those (as defaults or as-is) to build.

Or, you can build and install with

```bash
conan install . -of .  --build=missing
source build/Release/generators/conanbuild.sh
cmake --preset conan-release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build --preset conan-release --parallel
```

Then run with `./build/Release/grav`.
