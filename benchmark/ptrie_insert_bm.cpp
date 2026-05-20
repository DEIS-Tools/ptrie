#include "ptrie_insert.hpp"
#include "utils.h"

#include <ptrie/ptrie.h>

#include <sparsehash/dense_hash_set>
#include <sparsehash/sparse_hash_set>

#include <benchmark/benchmark.h>

#include <unordered_set>
#include <limits>

#include <cstdlib>  // rand
#include <cstddef>  // size_t
#include <cstdint>  // uint64_t

constexpr auto ELEM1 = 100;
constexpr auto ELEM2 = 1000;
constexpr auto ELEM3 = 10000;
constexpr auto ELEM4 = 100000;

using ptrie::wrapper_t;
using ptrie::hasher_o;
using ptrie::equal_o;

// NOLINTBEGIN(misc-use-anonymous-namespace,clang-analyzer-deadcode.DeadStores,cert-err58-cpp)

static void ptrie_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .bytes = 16,
                             .deletes = 0,
                             .read_rate = 2,
                             .maxval = 256};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = ptrie::set<>{};
        settings.seed = size_gen();
        set_insert_ptrie(set, settings);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(ptrie_bm)->Name("ptrie")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void std_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .bytes = 16,
                             .deletes = 0,
                             .read_rate = 2,
                             .maxval = 256};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = std::unordered_set<wrapper_t, hasher_o, equal_o>{};
        settings.seed = size_gen();
        set_insert(set, settings);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(std_bm)->Name("std")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void sparse_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .bytes = 16,
                             .deletes = 0,
                             .read_rate = 2,
                             .maxval = 256};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = google::sparse_hash_set<wrapper_t, hasher_o, equal_o>(settings.elements / 10);
        settings.seed = size_gen();
        set_insert(set, settings);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(sparse_bm)->Name("sparse")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void dense_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .bytes = 16,
                             .deletes = 0,
                             .read_rate = 2,
                             .maxval = 256};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = google::dense_hash_set<wrapper_t, hasher_o, equal_o>(settings.elements / 10);
        auto empty = wrapper_t{.data = {}, ._hash = 0};
        auto del = wrapper_t{.data = {}, ._hash = std::numeric_limits<uint64_t>::max()};
        set.set_empty_key(empty);
        settings.seed = size_gen();
        if (settings.deletes > 0.0)
            set.set_deleted_key(del);
        set_insert(set, settings);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(dense_bm)->Name("dense")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

// NOLINTEND(misc-use-anonymous-namespace,clang-analyzer-deadcode.DeadStores,cert-err58-cpp)
