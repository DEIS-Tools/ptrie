# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**ptrie** is a header-only C++20 library (LGPL v3+, version 1.1.3) implementing a memory-efficient, hash-free trie-based container with three variants:
- `ptrie::set<KEY>` — unordered set with variable-length keys
- `ptrie::map<KEY, T>` — map with variable-length keys and associated data
- `ptrie::set_stable<KEY>` / `ptrie::map<KEY, T>` (stable) — insertion-order-preserving variants accessible by index

All public headers live in `include/ptrie/`. There is no source to compile for the library itself.

## Build Commands

The project uses CMake presets. The presets are split between `CMakePresets.json` (top-level, includes `cmake/CMakePresets.json` and `cmake/CommonPresets.json`).

```bash
# Full build with sanitizers (ASAN + UBSAN + SSP) — typical dev workflow
cmake --preset multi-san
cmake --build --preset debug-san

# Run all tests
ctest --preset debug-san

# Quick header-only install (no tests/benchmarks)
cmake --workflow --preset quick-release
cmake --install build-quick --config Release --prefix=$PWD/local

# Full release with tests and benchmarks
cmake --workflow --preset release
```

## Running Tests

Test framework is **doctest** (fetched by CMake). Four test executables are built:

```bash
# Run all tests for a preset
ctest --preset debug-san --output-on-failure

# Run a single test executable directly (fastest)
./build-multi-san/Debug/Set
./build-multi-san/Debug/Map
./build-multi-san/Debug/Delete
./build-multi-san/Debug/StableSet
```

## Linting and Formatting

CI uses **clang-tidy-20**, **clang-format-20**, and **cmake-format**. To replicate locally:

```bash
# Check formatting (fails on any diff)
find . -iregex '.*\.\(c\|h\|cpp\|hpp\|cc\|hh\|cxx\|hxx\)$' | xargs clang-format-20 -n -Werror

# Apply formatting
find . -iregex '.*\.\(c\|h\|cpp\|hpp\|cc\|hh\|cxx\|hxx\)$' | xargs clang-format-20 -i

# Run clang-tidy (requires a build with compile commands)
run-clang-tidy-20 -p build-multi-san -header-filter="$PWD/(include|src|test|benchmark)/.*"
```

Format rules (`.clang-format`): Google style, 120-column limit, 4-space indent, K&R braces.

Tidy rules (`.clang-tidy`): most checks enabled; magic numbers, identifier length, cognitive complexity, and C-array usage are suppressed. Static constants use `UPPER_CASE`.

## Architecture

### Core Data Structure

The trie operates on **byte sequences** regardless of key type. Keys are decomposed into bytes via `byte_iterator<KEY>` (`include/ptrie/ptrie_memory.hpp`).

The trie has two node kinds:
- **`fwdnode_t`** — internal node with up to 256 children (one per byte value), indexed by a single byte of the key
- **`node_t`** (leaf node) — stores actual key suffixes and, for map variants, associated values

Memory layout is controlled by four template parameters on `ptrie_base` (`include/ptrie/ptrie_internal.hpp`):
- `HEAPBOUND` (default 17) — keys shorter than this are stored inline; longer keys spill to heap
- `SPLITBOUND` (default 129) — a node bucket splits into child nodes when it exceeds this count
- `BSIZE` (default 8) — bits used per trie level
- `ALLOCSIZE` (default 64KB) — slab allocation chunk size

### File Roles

| File | Role |
|---|---|
| `include/ptrie/ptrie.h` | Public API for `ptrie::set<KEY>` |
| `include/ptrie/ptrie_map.h` | Public API for `ptrie::set_stable` and `ptrie::map<KEY,T>` |
| `include/ptrie/ptrie_stable.h` | Stable map wrapper |
| `include/ptrie/ptrie_internal.hpp` | All trie logic — insertion, deletion, search, iteration (~68 KB) |
| `include/ptrie/ptrie_memory.hpp` | Unaligned memory ops, `byte_iterator<KEY>`, type-punning helpers |
| `include/ptrie/linked_bucket.h` | Lock-free atomic bucket chain; used by stable variants for O(1) indexed access |

### Stable Variants

`set_stable` / stable `map` maintain an insertion-order index. Each inserted element gets a persistent integer index. `linked_bucket_t` (`linked_bucket.h`) stores per-index metadata using atomic pointer chains, enabling concurrent appends.

### Iterators

All containers expose bidirectional iterators. Iteration traverses the trie in byte-lexicographic order; keys are **unpacked on demand** from the trie structure rather than stored explicitly.

### Namespacing

Internal implementation details live in the `ptrie::internal` namespace. Symbols previously using double-underscore prefixes were moved there (see recent commits).

## CI

Ten GitHub Actions workflows cover: Linux (GCC + Clang), macOS 14/15/26 (Apple Clang + GCC), and Windows (MSYS2 UCRT64). All builds use the `multi-san` preset (ASAN + UBSAN + SSP). A separate `tidy.yml` workflow enforces clang-format, clang-tidy, and cmake-format.

Each CI job also verifies the library can be consumed two ways: (1) via local install prefix, and (2) via CMake `FetchContent`.
