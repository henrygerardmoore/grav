# grav

https://github.com/henrygerardmoore/grav/assets/44307180/ba8dc1bc-df7a-4407-86e5-d2142297b42f

N-body gravity simulator using C++, EnTT, SFML, CMake, Quill, Eigen, and Conan


## Building
### Requirements
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

## Contributing
Feel free to open an issue (feature requests, bugs, or bad code that needs to be cleaned up) or make a pull request!
If you want to make a pull request, please [install pre-commit](https://pre-commit.com/#install) and run `pre-commit install` in the directory prior to making your pull request (until CI is set up).
