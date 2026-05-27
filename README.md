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

## Code Coverage
Configure with `-DPTRIE_COVERAGE=ON` and build the `coverage` target. It runs the
instrumented tests and produces an HTML report under `<build>/coverage-html` plus a
textual summary on the console. The tooling is selected by the compiler:
**gcov/lcov** for GCC, **llvm-cov** for Clang/AppleClang.
```shell
cmake --preset multi -DPTRIE_COVERAGE=ON
cmake --build --preset release-deb --target coverage
xdg-open build-multi/coverage-html/index.html  # or open ... on macOS
```
Coverage instrumentation is incompatible with the sanitizers, so use a non-sanitized
preset (e.g. `multi` / `release-deb`).

Extra tools are needed for the report (instrumentation alone works without them):
```shell
sudo apt install lcov          # GCC: lcov + genhtml
# Clang: llvm-cov and llvm-profdata ship with the LLVM toolchain
```

With Clang, `llvm-cov` prints `warning: N functions have mismatched data`. This is
benign: the library is header-only, so a templated function that is present but never
called in one test binary keeps a different structural hash than the instantiation
exercised in another. It therefore matches by name but not by hash when the merged
profile is applied across all test binaries. Coverage counts stay correct, and
`llvm-cov` has no flag to silence just this message (`--no-warn` does not affect it).
