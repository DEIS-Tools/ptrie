# Copilot instructions for ptrie

Purpose: concise, actionable reference for Copilot sessions and automated agents working in this repository.

---

## Build, test, and lint commands (explicit)

- Configure & build (common presets):
  - Quick header-only (no tests/benchmarks):
    cmake --workflow --preset quick-release
    cmake --install build-quick --config Release --prefix=$PWD/local

  - Full development workflow with sanitizers (typical):
    cmake --preset multi-san
    cmake --build --preset debug-san
    ctest --preset debug-san --output-on-failure

  - Full release (tests + benchmarks):
    cmake --workflow --preset release

- Run a single test executable (fastest):
  - After building a preset that produces test binaries, run the executable directly. Example paths used by CI/presets:
    ./build-multi-san/Debug/Set
    ./build-multi-san/Debug/Map
    ./build-multi-san/Debug/Delete
    ./build-multi-san/Debug/StableSet

  - ctest can run selected tests by regex from the build dir: ctest --test-dir <build-dir> -R <regex> --output-on-failure

- Coverage (non-sanitized presets):
  cmake --preset multi -DPTRIE_COVERAGE=ON
  cmake --build --preset release-deb --target coverage
  # report at build-multi/coverage-html/index.html

- Lint & formatting (CI uses clang-format-20 & clang-tidy-20):
  # check formatting (CI-failing)
  find . -iregex '.*\.(c|h|cpp|hpp|cc|hh|cxx|hxx)$' | xargs clang-format-20 -n -Werror

  # apply formatting
  find . -iregex '.*\.(c|h|cpp|hpp|cc|hh|cxx|hxx)$' | xargs clang-format-20 -i

  # clang-tidy (requires compilation DB from a build preset)
  run-clang-tidy-20 -p build-multi-san \
    -header-filter="$PWD/(include|src|test|benchmark)/.*" \
    -source-filter="$PWD/(src|test|benchmark)/.*"

---

## High-level architecture (big picture)

- Header-only C++20 library (no compiled sources). Public headers live in include/ptrie/.
- Core data structure is a byte-wise trie (keys decomposed via byte_iterator<KEY> in ptrie_memory.hpp).
- Two node kinds: fwdnode_t (internal, up to 256 children) and node_t (leaf storing suffixes and values).
- Memory layout and behavior controlled by template parameters on ptrie_base: HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE.
- Stable variants (set_stable / stable map) maintain insertion-order indexes using linked_bucket.h (lock-free bucket chain) to support O(1) indexed access and concurrent appends.
- Iteration is byte-lexicographic; keys are unpacked on demand from internal structures.
- Namespacing: implementation internals live under ptrie::internal.

Files of interest: include/ptrie/{ptrie.h, ptrie_map.h, ptrie_stable.h, ptrie_internal.hpp, ptrie_memory.hpp, linked_bucket.h} and test/ for doctest-based tests.

(See CLAUDE.md for a slightly more detailed breakdown; it was used when authoring this file.)

---

## Key conventions and repository-specific patterns

- Header-only pattern: consumers include ptrie via CMake package config (cmake/ptrie.cmake) or by installing the headers to a local prefix. Example usage lives under example/.

- CMake presets drive common workflows. Use cmake --list-presets and the provided quick/multi/multi-san presets. Quick presets disable tests/benchmarks for very fast local checks.

- Tests: doctest is used (doctest::doctest_with_main is linked into test executables). Test binaries are named Set, Map, Delete, StableSet and are intended to be runnable directly; add tests under test/ CMakeLists adds them.

- Coverage and heaptrack instrumentation: coverage target is available when configured; heaptrack tests are added via cmake/heaptrack.cmake but are disabled when sanitizers are active.

- Formatting/tidy in CI use clang-format-20 and clang-tidy-20 with the repository's .clang-format and .clang-tidy rules (Google style, 120-column limit, suppressed noisy checks). Local devs should match CI tool versions if possible.

- CMake options exposed in top-level CMakeLists.txt: PTRIE_BuildTests (ON/OFF), PTRIE_BuildBenchmark (ON/OFF), PTRIE_COVERAGE (ON/OFF). Use these when scripting builds.

- Tests and benchmarks are separated; quick presets turn them off to keep header-only workflows fast.

---

## Useful quick checks for Copilot sessions

- If asked how to run tests or produce coverage, prefer the CMake presets above (multi-san for sanitizer dev flow, multi/release for coverage).
- If asked to run or diagnose a single failing test, build the debug or debug-san target and run the corresponding test executable directly; ctest -R is an alternative.
- When editing core algorithms, pay attention to include/ptrie/ptrie_internal.hpp (single large implementation file ~68KB) and ensure template changes don't break compile for all test binaries.

---

## Where to look for further policy/config files

- CLAUDE.md: repository-specific guidance used to author these instructions. Keep in sync if architecture or workflows change.
- CI workflows (/.github/workflows) define exact CI tool versions and steps — consult them for reproducing CI locally.

---

If you'd like, update this file to add more focussed instructions (examples: how to add a new test, how to run clang-tidy with a specific check list, or how to reproduce a CI job locally).