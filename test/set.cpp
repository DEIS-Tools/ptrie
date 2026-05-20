/*
 * Copyright Peter G. Jensen <root@petergjoel.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// Created by Peter G. Jensen on 12/9/16.

#include "utils.h"
#include <ptrie/ptrie.h>
#include <ptrie/ptrie_memory.hpp>

#include <doctest/doctest.h>

#include <vector>
#include <limits>

#include <cstring>  // memcpy
#include <cstddef>  // size_t

TEST_SUITE_BEGIN("PTrie Set");

using ptrie::uchar;

TEST_CASE("Empty")
{
    auto set = typename ptrie::set<>{};
    try_insert(set, [](size_t) { return std::vector<uchar>{}; }, 1);
}

TEST_CASE("Insert Byte")
{
    auto set = ptrie::set<>{};
    try_insert(set, [](size_t i) { return std::vector{static_cast<uchar>(i)}; }, 256);
}

TEST_CASE("Insert Byte Split")
{
    auto set = ptrie::set<uchar, 128, 6>{};
    try_insert(set, [](size_t i) { return std::vector{static_cast<uchar>(i)}; }, 256);
}

TEST_CASE("Heap Test")
{
    auto set = ptrie::set<uchar, sizeof(size_t) + 1>{};
    try_insert(
        set,
        [](size_t i) {
            auto data = std::vector<uchar>(sizeof(size_t));
            std::memcpy(std::data(data), &i, sizeof(size_t));
            return data;
        },
        1024);
}

TEST_CASE("Insert Mill")
{
    auto set = ptrie::set<>{};
    try_insert(
        set,
        [](size_t i) {
            auto data = std::vector<uchar>(sizeof(size_t));
            std::memcpy(std::data(data), &i, sizeof(size_t));
            return data;
        },
        1024_uz * 1024);
}

TEST_CASE("Pseudo Rand1")
{
    for (size_t seed = 0; seed < 10; ++seed) {
        auto set = ptrie::set<>{};
        try_insert(set, [seed](size_t i) { return rand_data(seed + i, 256); }, 1024_uz * 10u);
    }
}

TEST_CASE("Pseudo Rand2")
{
    for (size_t seed = 0; seed < 10; ++seed) {
        auto set = ptrie::set<>{};
        try_insert(set, [seed](size_t i) { return rand_data(seed + i, 16); }, 1024_uz * 10u);
    }
}

TEST_CASE("Pseudo Rand Split Heap")
{
    for (size_t seed = 42; seed < (42 + 10); ++seed) {
        auto set = ptrie::set<uchar, sizeof(size_t) + 1, 6>{};
        try_insert(set, [seed](size_t i) { return rand_data(seed + i, 16); }, 1024_uz * 10u);
    }
}

TEST_CASE("Simple Copy")
{
    auto set = ptrie::set<size_t>{};
    for (size_t i = 0; i < 100000; ++i)
        set.insert(i);
    {
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        const auto cpy = set;  // copy on purpose
        size_t i = 0;
        for (; i < 100000; ++i)
            REQUIRE(cpy.exists(i).first);
        for (; i < 200000; ++i)
            REQUIRE(!cpy.exists(i).first);
    }
}

TEST_CASE("Simple Iterator Invariant")
{
    const auto set = ptrie::set<size_t>{};
    REQUIRE(set.begin() == set.end());
}

TEST_CASE("Single Iterator")
{
    const auto set = [] {
        auto res = ptrie::set<size_t>{};
        res.insert(1);
        return res;
    }();
    auto b = set.begin();
    auto e = set.end();
    REQUIRE(b != e);
    ++b;
    REQUIRE(b == e);
    --b;
    REQUIRE(b == set.begin());
    ++b;
    REQUIRE(b == set.end());
    --e;
    REQUIRE(e == set.begin());
}

TEST_CASE("Simple Iterator")
{
    const auto set = [] {
        auto res = ptrie::set<size_t>{};
        for (size_t i = 0; i < 100000; ++i)
            res.insert(i);
        return res;
    }();
    size_t cnt = 0;
    for (auto b = set.begin(); b != set.end(); ++b)
        ++cnt;
    CHECK(cnt == size_t{100000});
}

TEST_CASE("Ranged-for-loop")
{
    const auto set = [] {
        auto res = ptrie::set<int>{};
        for (auto i = -127; i < 128; ++i)
            if (i % 3 == 0)
                res.insert(i);
        return res;
    }();
    for (auto&& value : set)
        CHECK(value % 3 == 0);
}

TEST_CASE("Simple RIterator")
{
    const auto set = [] {
        auto res = ptrie::set<size_t>{};
        for (size_t i = 0; i < 100000; ++i)
            res.insert(i);
        return res;
    }();
    size_t cnt = 0;
    for (auto b = --set.end(); b != set.begin(); --b)
        ++cnt;
    CHECK(cnt == size_t{100000 - 1});
}

TEST_CASE("Dealloc")
{
    auto set = ptrie::set<>{};
    auto mem = std::vector<std::vector<uchar>>{};
    mem.reserve(10000);
    for (size_t i = 1; i < 10000; ++i) {
        const auto* tmp = mem.emplace_back(i, std::numeric_limits<uchar>::max()).data();
        set.insert(tmp, i);
    }
}

TEST_CASE("New Node At Depth One")
{
    // Exercises ptrie_internal.hpp:1252 where node = new node_t{} is allocated
    // with byte == 1, i.e. base == fwd at a depth-1 fwd node.
    //
    // Setup (SPLITBOUND=6, HEAPBOUND=17, BSIZE=8):
    //   Six 128-byte keys share the same root slot (lengthc[1]=0) and fill one
    //   leaf.  In split_fwd at p_byte=0, each key has f[0]=0x80 (low byte of
    //   length=128), so b = 0x80 & 0x80 = 0x80 > 0: all go to the HIGH
    //   partition (lcnt=0).  split_fwd therefore places the existing leaf in
    //   fwd_n->_children[128..255] and stores self-loops in
    //   fwd_n->_children[0..127].
    //
    //   The 7th insert has length 5 (nitemsize = 5-1 = 4 < HEAPBOUND=17,
    //   copyval=true).  fast_forward traverses root -> fwd_n and finds
    //   fwd_n->_children[5] = fwd_n (self-loop), so base == fwd, byte == 1.
    //   The do-while at line 1265 finds an occupied sibling in the high half
    //   and narrows min=max=5; the for loop at 1281 runs once and stores the
    //   new node at fwd_n->_children[5].  ASAN confirms the node is freed.
    auto set = ptrie::set<uchar, 17, 6>{};

    for (uchar i = 0; i < 6; ++i) {
        const auto key = std::vector<uchar>(128, i);
        REQUIRE(set.insert(key.data(), key.size()).first);
    }
    const auto short_key = std::vector<uchar>(5, static_cast<uchar>(0x42));
    REQUIRE(set.insert(short_key.data(), short_key.size()).first);

    for (uchar i = 0; i < 6; ++i) {
        const auto key = std::vector<uchar>(128, i);
        REQUIRE(set.exists(key.data(), key.size()).first);
    }
    REQUIRE(set.exists(short_key.data(), short_key.size()).first);
}

TEST_CASE("Split Node Both Partitions")
{
    // Exercises split_node at ptrie_internal.hpp:1154 where h_node is allocated
    // because both the low and high partitions of a split are non-empty.
    //
    // How the mixed split arises (SPLITBOUND=6, HEAPBOUND=17, BSIZE=8):
    //   The six keys all have lengthc[1]=0 so they share one root leaf.
    //   The 6th insert (length-4, nitemsize=4 < HEAPBOUND, copyval=true)
    //   brings count to SPLITBOUND and triggers split_node at p_byte=0.
    //   At p_byte=0 all keys have f[1]=0 so every split bit is 0; after 8
    //   single-partition recursions split_fwd fires and creates a fwd at
    //   p_byte=0 with node->_type reset to 1.  The recursive split_node at
    //   p_byte=1 (r_pos=1) tests bit 6 of f[1]=length:
    //     length-64 keys: f[1]=0x40, (0x40 & _masks[1]=0x40) > 0 -> HIGH (hcnt=5)
    //     length-4  key:  f[1]=0x04, (0x04 & 0x40) == 0           -> LOW  (lcnt=1)
    //   Both non-zero => else-branch => h_node = new node_t{} (line 1154).
    //   h_node is linked into jumppar->_children; ASAN confirms it is freed.
    auto set = ptrie::set<uchar, 17, 6>{};

    for (uchar i = 0; i < 5; ++i) {
        const auto key = std::vector<uchar>(64, i);
        REQUIRE(set.insert(key.data(), key.size()).first);
    }
    const auto short_key = std::vector<uchar>(4, static_cast<uchar>(0x99));
    REQUIRE(set.insert(short_key.data(), short_key.size()).first);

    for (uchar i = 0; i < 5; ++i) {
        const auto key = std::vector<uchar>(64, i);
        REQUIRE(set.exists(key.data(), key.size()).first);
    }
    REQUIRE(set.exists(short_key.data(), short_key.size()).first);
}

TEST_CASE("Heap Key Lifecycle")
{
    // Exercises ptrie_internal.hpp:1386 where nenc_size >= HEAPBOUND causes the
    // key suffix to be heap-allocated via new_uchar and its raw pointer to be
    // embedded inside the node bucket via mem_store (line 1396).  The destructor
    // must recover every such allocation; ASAN on the debug-san preset will catch
    // any leak.  Keys of 20 bytes exceed the default HEAPBOUND of 17.
    //
    // All 20-byte keys share the same root fwd-slot (the second byte of the
    // encoded length is 0x00 for lengths 0-255), so they land in one leaf node.
    // Inserting 10 distinct keys drives the heap path at b_index 0 through 9.
    constexpr size_t key_len = 20;
    auto set = ptrie::set<>{};
    try_insert(set, [](size_t i) { return std::vector<uchar>(key_len, static_cast<uchar>(i)); }, 10);
    // Destructor of set frees the 10 heap-allocated key suffixes.
}

TEST_CASE("Iterator Dereference Value")
{
    // Exercises iterator_base::operator* at ptrie_internal.hpp:197 via
    // write_data<..., int, 1, 8, 17>.  clang-tidy flags operator* as a
    // potential uninitialized return: in the ps=1 branch of build_path,
    // size is recovered as (stored_uint16 >> 8), and the analyzer admits
    // size=0 in its symbolic model.  With size=0 both writes inside
    // `if (ps > 0)` are guarded by `pos < size` which evaluates false,
    // leaving *dest unwritten and key returned as garbage (line 197).
    //
    // In practice the stored uint16 for an int key encodes the key length
    // (always 4), so (stored >> 8) == 0x04 after one fwd level and write_data
    // always writes all four key bytes.  Inserting 200 ints (> default
    // SPLITBOUND=129) forces two fwd splits, producing nodes at ps=2 that
    // exercise the write_data code path.  UBSAN catches any garbage read via
    // the set.exists() call on each dereferenced value.
    auto set = ptrie::set<int>{};
    constexpr int N = 200;
    for (int i = 0; i < N; ++i)
        set.insert(i);
    size_t cnt = 0;
    for (auto&& value : set) {
        CHECK(set.exists(value).first);
        ++cnt;
    }
    CHECK(cnt == size_t{N});
}
