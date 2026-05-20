#include "int_ptrie_insert.hpp"
#include "utils.h"

#include <ptrie/ptrie.h>
#include <ptrie/ptrie_stable.h>
#include <ptrie/ptrie_map.h>

#include <benchmark/benchmark.h>

#include <sparsehash/dense_hash_set>
#include <sparsehash/sparse_hash_set>

#include <vector>
#include <set>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <numeric>  // iota
#include <limits>

#include <cstddef>  // size_t
#include <cstdint>  // uint32_t

// NOLINTBEGIN(misc-use-anonymous-namespace,clang-analyzer-deadcode.DeadStores,cert-err58-cpp)

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)
static size_t seed = std::random_device{}();
constexpr double deletes = 0.0;
constexpr double read_rate = 2.0;

static_assert(read_rate > 0, "normal distribution requires deviation greater than zero");

static const auto order = [] {  // NOLINT(cert-err58-cpp)
    auto res = std::vector<size_t>(sizeof(size_t) * 8);
    std::iota(res.begin(), res.end(), 0);
    auto gen = std::default_random_engine(seed);
    std::shuffle(res.begin(), res.end(), gen);
    return res;
}();

constexpr auto ELEM1 = 100;
constexpr auto ELEM2 = 1000;
constexpr auto ELEM3 = 10000;
constexpr auto ELEM4 = 100000;

static void ptrie_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .deletes = deletes,
                             .read_rate = read_rate};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = ptrie::set<>{};
        settings.seed = size_gen();
        set_insert_ptrie(set, settings, order);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(ptrie_bm)->Name("ptrie")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void ptrie_stable_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .deletes = deletes,
                             .read_rate = read_rate};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = ptrie::set_stable<>{};
        settings.seed = size_gen();
        set_insert_ptrie(set, settings, order);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(ptrie_stable_bm)->Name("ptrie-stable")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void ptrie_map_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .deletes = deletes,
                             .read_rate = read_rate};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = ptrie::map<unsigned char, size_t>{};
        settings.seed = size_gen();
        set_insert_ptrie(set, settings, order);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(ptrie_map_bm)->Name("ptrie-map")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void std_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .deletes = deletes,
                             .read_rate = read_rate};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = std::unordered_set<size_t, hasher_o, equal_o>{};
        settings.seed = size_gen();
        set_insert(set, settings, order);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(std_bm)->Name("std")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void redblack_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .deletes = deletes,
                             .read_rate = read_rate};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = std::set<size_t>{};
        settings.seed = size_gen();
        set_insert(set, settings, order);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(redblack_bm)->Name("redblack")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void sparse_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .deletes = deletes,
                             .read_rate = read_rate};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = google::sparse_hash_set<size_t, hasher_o, equal_o>(settings.elements / 10);
        settings.seed = size_gen();
        set_insert(set, settings, order);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(sparse_bm)->Name("sparse")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

static void dense_bm(benchmark::State& state)
{
    auto settings = Settings{.type = state.name(),
                             .elements = static_cast<size_t>(state.range(0)),
                             .deletes = deletes,
                             .read_rate = read_rate};
    auto size_gen = rand_gen<size_t>();
    for (auto _ : state) {
        auto set = google::dense_hash_set<size_t, hasher_o, equal_o>(settings.elements / 10);
        set.set_empty_key(reorder(0, order, seed));
        settings.seed = size_gen();
        if (settings.deletes > 0.0)
            set.set_deleted_key(reorder(std::numeric_limits<uint32_t>::max(), order, seed));
        set_insert(set, settings, order);
        benchmark::DoNotOptimize(set);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(dense_bm)->Name("dense")->Arg(ELEM1)->Arg(ELEM2)->Arg(ELEM3)->Arg(ELEM4);

// NOLINTEND(misc-use-anonymous-namespace,clang-analyzer-deadcode.DeadStores,cert-err58-cpp)