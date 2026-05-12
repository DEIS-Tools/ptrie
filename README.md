# ptrie
A memory efficient hashfree hashmap implementation packaged as a *header-only* library.

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
ctest --test-dir build-example-local --output-on-failure
```

Test by fetching ptrie from the repository using [ptrie.cmake](cmake/ptrie.cmake):
```shell
cmake -S example -B build-example && \
cmake --build build-example && \
ctest --test-dir build-example --output-on-failure
```
Clean all:
```shell
rm -Rf local build-quick build-example-local build-example
```



# Testing and Benchmarking

## Dependencies
Ubuntu (assuming 24.04 LTS):
```shell
sudo apt install cmake ninja-build g++
sudo apt install libsparsehash-dev
```
macOS (install [XCode from AppStore](https://apps.apple.com/us/app/xcode/id497799835), and [Homebrew](https://brew.sh)):
```shell
brew install cmake ninja 
brew install google-sparsehash
```

## Configure, Build, Test and Benchmark
Test and benchmark release:
```shell
cmake --workflow --preset release
```
Test Debug with Sanitizers:
```shell
cmake --workflow --preset debug-san
```
Inspect other presets:
```shell
cmake --workflow --list-presets
```
```shell
cmake --list-presets=configure
cmake --list-presets=build
cmake --list-presets=test
```