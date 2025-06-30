# ptrie
A memory efficient hashfree hashmap implementation packaged as a header-only library.

Use CMake to install to `$PWD/local` (headers and CMake configuration):
```shell
cmake -B build -DCMAKE_BUILD_TYPE=Release -DPTRIE_BuildTests=OFF -DPTRIE_BuildBenchmark=OFF
cmake --build build
cmake --install build --prefix=$PWD/local
```

# Testing and Benchmarking

## Dependencies for Ubuntu

```shell
sudo apt install cmake ninja-build g++
sudo apt install libsparsehash-dev libboost-test-dev
```

## Configure, Build, Test and Benchmark:
Test and benchmark release:
```shell
cmake --workflow dev
```
Test Debug with Sanitizers:
```shell
cmake --workflow dev-san
```
