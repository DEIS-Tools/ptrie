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

rm -Rf local build-quick-san build-example-local build-example

show "Create a quick release and install it into $PWD/local:"
cmake --workflow --preset quick-san
cmake --install build-quick-san --config Debug --prefix=$PWD/local

show "Configure, build and run example against ptrie installed in $PWD/local:"
cmake -S example -B build-example-local -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=$PWD/local
cmake --build build-example-local --config Debug
ctest --test-dir build-example-local -C Debug --output-on-failure

PTRIE_BRANCH=$(git rev-parse --abbrev-ref HEAD)
REMOTE_NAME=$(git config branch.$PTRIE_BRANCH.remote)
if [ -z "$REMOTE_NAME" ]; then
  show "The branch is not pushed to remote repo, skipping the example test."
  exit 0
fi
git status -uno | grep -q "Your branch is up to date with " || show "WARNING: the branch is not up-to-date with the remote repo."
PTRIE_REPO=$(git remote get-url "$REMOTE_NAME")
show "Trying example against ptrie from branch '$PTRIE_BRANCH' at '$PTRIE_REPO':"
cmake -S example -B build-example -DCMAKE_BUILD_TYPE=Debug -DPTRIE_REPO="$PTRIE_REPO" -DPTRIE_BRANCH="$PTRIE_BRANCH"
cmake --build build-example --config Debug
ctest --test-dir build-example -C Debug --output-on-failure
