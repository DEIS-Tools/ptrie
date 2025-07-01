# ptrie
A memory efficient hashfree hashmap implementation packaged as a header-only library.

Use CMake to install to `$PWD/local` (headers and CMake configuration):
```shell
cmake --workflow --preset quick-release
cmake --install build-quick --config Release --prefix=$PWD/local
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