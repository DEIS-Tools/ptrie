# ptrie
A memory efficient hashfree hashmap implementation packaged as a header-only library.

# Testing and Benchmarking

## Dependencies for Ubuntu

```shell
sudo apt install cmake ninja-build g++
sudo apt install libsparsehash-dev libboost-test-dev
```

## Configure, Build and Test
Test and benchmark release:
```shell
cmake --workflow dev
```
Test Debug with Sanitizers:
```shell
cmake --workflow dev-san
```