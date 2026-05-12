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

#include <doctest/doctest.h>

#include <vector>

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
            memcpy(std::data(data), &i, sizeof(size_t));
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
            memcpy(std::data(data), &i, sizeof(size_t));
            return data;
        },
        1024 * 1024);
}

TEST_CASE("Pseudo Rand1")
{
    for (size_t seed = 0; seed < 10; ++seed) {
        auto set = ptrie::set<>{};
        try_insert(set, [seed](size_t i) { return rand_data(seed + i, 256); }, 1024 * 10);
    }
}

TEST_CASE("Pseudo Rand2")
{
    for (size_t seed = 0; seed < 10; ++seed) {
        auto set = ptrie::set<>{};
        try_insert(set, [seed](size_t i) { return rand_data(seed + i, 16); }, 1024 * 10);
    }
}

TEST_CASE("Pseudo Rand Split Heap")
{
    for (size_t seed = 42; seed < (42 + 10); ++seed) {
        auto set = ptrie::set<uchar, sizeof(size_t) + 1, 6>{};
        try_insert(set, [seed](size_t i) { return rand_data(seed + i, 16); }, 1024 * 10);
    }
}

TEST_CASE("Simple Copy")
{
    auto set = ptrie::set<size_t>{};
    for (size_t i = 0; i < 100000; ++i) {
        set.insert(i);
    }
    {
        const auto cpy = set;
        size_t i = 0;
        for (; i < 100000; ++i)
            REQUIRE(cpy.exists(i).first);
        for (; i < 200000; ++i)
            REQUIRE(!cpy.exists(i).first);
    }
}

TEST_CASE("Simple Iterator Invariant")
{
    auto set = ptrie::set<size_t>{};
    REQUIRE(set.begin() == set.end());
}

TEST_CASE("Single Iterator")
{
    auto set = ptrie::set<size_t>{};
    set.insert(1);
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
    auto set = ptrie::set<size_t>{};
    for (size_t i = 0; i < 100000; ++i)
        set.insert(i);
    size_t cnt = 0;
    for (auto b = set.begin(); b != set.end(); ++b)
        ++cnt;
    CHECK(cnt == size_t{100000});
}

TEST_CASE("Ranged-for-loop")
{
    auto set = ptrie::set<int>{};
    for (auto i = -127; i < 128; ++i)
        if (i % 3 == 0)
            set.insert(i);
    for (auto&& value : set)
        CHECK(value % 3 == 0);
}

TEST_CASE("Simple RIterator")
{
    auto set = ptrie::set<size_t>{};
    for (size_t i = 0; i < 100000; ++i)
        set.insert(i);
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
