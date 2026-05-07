#include "ptrie_insert.hpp"
#include "utils.h"

#include <ptrie/ptrie.h>

#include <sparsehash/dense_hash_set>
#include <sparsehash/sparse_hash_set>

#include <benchmark/benchmark.h>

#include <unordered_set>

constexpr size_t seed = 0;
constexpr double deletes = 0.0;
constexpr double read_rate = 2.0;
constexpr size_t maxval = 256;
constexpr size_t bytes = 16;

static_assert(read_rate > 0, "normal distribution requires deviation greater than zero");

constexpr auto ELEM1 = 100;
constexpr auto ELEM2 = 1000;
constexpr auto ELEM3 = 10000;
constexpr auto ELEM4 = 100000;

using ptrie::wrapper_t;
using ptrie::hasher_o;
using ptrie::equal_o;

static void ptrie_bm(benchmark::State& state)
{
    const auto elements = state.range(0);
    for (auto _ : state) {
        auto set = ptrie::set<>{};
        set_insert_ptrie(set, elements, seed, bytes, deletes, read_rate, maxval);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(ptrie_bm)->Name("ptrie")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void std_bm(benchmark::State& state)
{
    const auto elements = state.range(0);
    for (auto _ : state) {
        auto set = std::unordered_set<wrapper_t, hasher_o, equal_o>{};
        set_insert(set, elements, seed, bytes, deletes, read_rate, maxval);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(std_bm)->Name("std")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void sparse_bm(benchmark::State& state)
{
    const auto elements = state.range(0);
    for (auto _ : state) {
        auto set = google::sparse_hash_set<wrapper_t, hasher_o, equal_o>(elements / 10);
        set_insert(set, elements, seed, bytes, deletes, read_rate, maxval);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(sparse_bm)->Name("sparse")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void dense_bm(benchmark::State& state)
{
    const auto elements = state.range(0);
    for (auto _ : state) {
        auto set = google::dense_hash_set<wrapper_t, hasher_o, equal_o>(elements / 10);
        auto empty = wrapper_t{{}, 0};
        auto del = wrapper_t{{}, std::numeric_limits<uint64_t>::max()};
        set.set_empty_key(empty);
        if (deletes > 0.0)
            set.set_deleted_key(del);
        set_insert(set, elements, seed, bytes, deletes, read_rate, maxval);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(dense_bm)->Name("dense")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);
