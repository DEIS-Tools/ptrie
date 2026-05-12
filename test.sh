#!/usr/bin/env bash
set -e

function show() {
  echo
  echo "$(tput bold)$@$(tput sgr0)"
}

if [ -n "$PTRIE_BuildTests" ]; then
  rm -Rf build-multi-san
  show "Configure, build and run tests with debug and sanitizers:"
  cmake --workflow --preset debug-san
fi

rm -Rf local build-quick build-example-local build-example

show "Create a quick release and install it into $PWD/local:"
cmake --workflow --preset quick-release
cmake --install build-quick --config Release --prefix=$PWD/local

show "Configure, build and run example against ptrie installed in $PWD/local:"
cmake -S example -B build-example-local -DCMAKE_PREFIX_PATH=$PWD/local
cmake --build build-example-local
ctest --test-dir build-example-local --output-on-failure

show "Configure, build and run example against ptrie fetched from repository:"
cmake -S example -B build-example
cmake --build build-example
ctest --test-dir build-example --output-on-failure
