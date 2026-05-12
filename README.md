# ptrie
A memory efficient hashfree hashmap implementation packaged as a *header-only* library.

## Dependencies
Ubuntu (assuming 24.04 LTS):
```shell
sudo apt install cmake ninja-build g++
sudo apt install libsparsehash-dev
```
macOS (install [XCode from AppStore](https://apps.apple.com/us/app/xcode/id497799835), and [Homebrew](https://brew.sh)) using native AppleClang:
```shell
brew install cmake ninja  
brew install google-sparsehash
```
macOS (install [XCode from AppStore](https://apps.apple.com/us/app/xcode/id497799835), and [Homebrew](https://brew.sh)) using GCC from brew:
```shell
brew install cmake ninja gcc
brew install google-sparsehash
export CC=gcc-15
export CXX=g++-15
```

Windows [MSYS2](https://msys2.org) UCRT64:
```shell
PRE=mingw-w64-ucrt-x86_64
pacman -S $PRE-git $PRE-cmake $PRE-ninja $PRE-toolchain $PRE-sparsehash
```

## Installation
Use CMake to install to `$PWD/local` (headers and CMake configuration):
```shell
cmake --workflow --preset quick-release && \
cmake --install build-quick --config Release --prefix=$PWD/local
```

## Example Usage
Directory [example](example) contains a sample project using ptrie.

Test against the ptrie installed in [local](local) using [ptrie.cmake](cmake/ptrie.cmake):
```shell
cmake -S example -B build-example-local -DCMAKE_PREFIX_PATH=$PWD/local && \
cmake --build build-example-local && \
ctest --test-dir build-example-local --verbose
```

Test by fetching ptrie from the repository using [ptrie.cmake](cmake/ptrie.cmake):
```shell
cmake -S example -B build-example && \
cmake --build build-example && \
ctest --test-dir build-example --verbose
```
Clean all:
```shell
rm -Rf local build-quick build-example-local build-example
```


## Testing and Benchmarking

Benchmark results are plotted using python when the following packages are installed:
```shell
sudo apt install python3-matplotlib python3-pandas python3-scipy python3-pyqt6
```
Test and benchmark a release build:
```shell
cmake --workflow --preset release
```
Test and debug with sanitizers:
```shell
cmake --workflow --preset debug-san
```
Inspect other workflow presets:
```shell
cmake --workflow --list-presets
```
See also configuration, build and test presets:
```shell
cmake --list-presets=configure
cmake --list-presets=build
cmake --list-presets=test
```
For example multi-config with sanitizers Debug build and test:
```shell
cmake --preset multi-san
cmake --build --preset debug-san
ctest --preset debug-san
```
