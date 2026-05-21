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

#include <ptrie/ptrie_stable.h>
#include <ptrie/ptrie_memory.hpp>  // uchar

#include <doctest/doctest.h>

#include <iostream>
#include <vector>
#include <algorithm>    // equal
#include <type_traits>  // has_unique_object_representations_v

#include <cstring>  // memcmp
#include <cassert>
#include <cstddef>  // size_t

TEST_SUITE_BEGIN("PTrie Stable Set");

using ptrie::uchar;

TEST_CASE("Simple RIterator")
{
    constexpr size_t x = 10000;
    auto set = ptrie::set_stable<size_t>{};
    for (size_t i = 0; i < x; ++i)
        set.insert(i);
    size_t cnt = 0;
    for (auto b = --set.end(); b != set.begin(); --b) {
        ++cnt;
        REQUIRE(b.index() <= x);
        REQUIRE(b.index() == b.unpack().back());
    }
    CHECK(cnt == x - 1);
}

TEST_CASE("Simple Iterator")
{
    constexpr size_t x = 10000;
    auto set = ptrie::set_stable<size_t>{};
    for (size_t i = 0; i < x; ++i) {
        set.insert(i);
    }
    size_t cnt = 0;
    for (auto b = set.begin(); b != set.end(); ++b) {
        ++cnt;
        REQUIRE(b.index() <= x);
        REQUIRE(b.index() == b.unpack().back());
    }
    CHECK(cnt == x);
}

TEST_CASE("Ranged-for-loop")
{
    auto set = ptrie::set_stable<int>{};
    for (auto i = -127; i < 128; ++i)
        if (i % 3 == 0)
            set.insert(i);
    for (auto&& value : set)
        CHECK(value % 3 == 0);
}

TEST_CASE("Pseudo Rand1")
{
    for (size_t seed = 1337; seed < (1337 + 10); ++seed) {
        auto set = ptrie::set_stable<>{};
        auto ids = std::vector<size_t>{};
        auto scratchpad = std::vector<uchar>(20 + sizeof(size_t));
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto data = rand_data(i + seed, 20);
            const auto [res, id] = set.insert(std::data(data), std::size(data));
            CHECK(res);
            ids.push_back(id);
            const auto size = set.unpack(id, std::data(scratchpad));

            CHECK(std::size(data) == size);
            CHECK(memcmp(std::data(data), std::data(scratchpad), size) == 0);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto data = rand_data(i + seed, 20);
            const auto size = set.unpack(ids[i], std::data(scratchpad));

            CHECK(std::size(data) == size);
            CHECK(memcmp(std::data(data), std::data(scratchpad), size) == 0);

            const auto key = set.unpack(ids[i]);
            REQUIRE(std::size(data) == key.size());
            CHECK(memcmp(std::data(data), std::data(key), size) == 0);
        }
    }
}

TEST_CASE("Pseudo Rand Split Heap")
{
    for (size_t seed = 42; seed < (42 + 10); ++seed) {
        auto set = ptrie::set_stable<uchar, size_t, sizeof(size_t) + 1, 6>{};
        auto ids = std::vector<size_t>{};
        auto scratchpad = std::vector<uchar>(20 + sizeof(size_t));

        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto data = rand_data(i + seed, 20);
            const auto [res, id] = set.insert(std::data(data), std::size(data));
            CHECK(res);
            ids.push_back(id);
            const auto size = set.unpack(id, std::data(scratchpad));

            CHECK(std::size(data) == size);
            CHECK(memcmp(std::data(data), std::data(scratchpad), size) == 0);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto data = rand_data(i + seed, 20);
            const auto size = set.unpack(ids[i], std::data(scratchpad));

            CHECK(std::size(data) == size);
            CHECK(memcmp(std::data(data), std::data(scratchpad), size) == 0);

            const auto key = set.unpack(ids[i]);
            CHECK(std::size(data) == key.size());
            CHECK(memcmp(std::data(data), key.data(), size) == 0);
        }
    }
}

/// Input type to byte_iterator with padding
struct type_t
{
    char _a;
    int _b;
    char _c;
    int _d;
    bool operator==(const type_t& other) const noexcept = default;
    static type_t rand(unsigned int seed)
    {
        auto char_gen = rand_gen<char>();
        auto int_gen = rand_gen<int>(seed);
        return {._a = char_gen(), ._b = int_gen(), ._c = char_gen(), ._d = int_gen()};
    }
    static std::vector<type_t> rand_vec(unsigned int seed, std::size_t size)
    {
        auto char_gen = rand_gen<char>();
        auto int_gen = rand_gen<int>(seed);
        auto res = std::vector<type_t>(size);
        for (auto& t : res)
            t = type_t{._a = char_gen(), ._b = int_gen(), ._c = char_gen(), ._d = int_gen()};
        return res;
    }
    friend std::ostream& operator<<(std::ostream& os, const type_t& el)
    {
        std::cerr << +el._a << ", " << el._b << ", " << +el._c << ", " << el._d;
        return os;
    }
};
static_assert(!std::has_unique_object_representations_v<type_t>);

template <>
struct ptrie::byte_iterator<type_t>
{
    static constexpr uchar& access(type_t* data, size_t id)
    {
        auto el = id / element_size();
        id = id % element_size();
        assert(id < element_size());

        switch (id) {
        case 0: return *as_array(&data[el]._a);
        case 1:
        case 2:
        case 3:
        case 4: return as_array(&data[el]._b)[id - 1];
        case 5: return *as_array(&data[el]._c);
        default: return as_array(&data[el]._d)[id - 6];
        }
    }

    static constexpr const uchar& const_access(const type_t* data, size_t id)
    {
        return access(const_cast<type_t*>(data), id);
    }

    static constexpr size_t element_size()
    {
        return sizeof(type_t::_a) + sizeof(type_t::_b) + sizeof(type_t::_c) + sizeof(type_t::_d);
    }

    static constexpr bool continuous() { return false; }
    [[deprecated("wrong spelling")]] static constexpr bool continious() { return false; }
};

TEST_CASE("Complex Type1")
{
    for (size_t seed = 1337; seed < (1337 + 10); ++seed) {
        auto set = ptrie::set_stable<type_t>{};
        auto ids = std::vector<size_t>{};
        auto scratchpad = type_t{};
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto test = type_t::rand(i + seed);
            const auto [res, id] = set.insert(test);
            CHECK(res);
            ids.push_back(id);
            const auto size = set.unpack(id, &scratchpad);

            CHECK(size_t{1} == size);
            CHECK(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto test = type_t::rand(i + seed);
            const auto size = set.unpack(ids[i], &scratchpad);

            CHECK(size_t{1} == size);
            CHECK(test == scratchpad);

            const auto key = set.unpack(ids[i]);
            CHECK(size_t{1} == key.size());
            CHECK(key.back() == test);
        }
    }
}

TEST_CASE("Complex Type2")
{
    for (size_t seed = 1337; seed < (1337 + 10); ++seed) {
        auto set = ptrie::set_stable<type_t, size_t, 9>{};
        auto ids = std::vector<size_t>{};
        auto scratchpad = type_t{};
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto test = type_t::rand(i + seed);
            const auto [res, id] = set.insert(test);
            REQUIRE(res);
            ids.push_back(id);
            const auto size = set.unpack(id, &scratchpad);

            REQUIRE(size == 1);
            REQUIRE(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto test = type_t::rand(i + seed);
            const auto size = set.unpack(ids[i], &scratchpad);

            CHECK(size == 1);
            CHECK(test == scratchpad);

            const auto key = set.unpack(ids[i]);
            CHECK(key.size() == 1);
            CHECK(key.back() == test);
        }
    }
}

TEST_CASE("Complex Type1 Vector")
{
    for (size_t seed = 1337; seed < (1337 + 10); ++seed) {
        auto set = ptrie::set_stable<type_t>{};
        auto ids = std::vector<size_t>{};
        auto scratchpad = std::vector<type_t>(10);
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto test = type_t::rand_vec(i + seed, 10);
            const auto [res, id] = set.insert(test);
            REQUIRE(res);
            ids.push_back(id);
            const auto size = set.unpack(id, scratchpad.data());

            REQUIRE(size == 10);
            REQUIRE(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto test = type_t::rand_vec(i + seed, 10);
            const auto size = set.unpack(ids[i], scratchpad.data());

            CHECK(size == 10);
            CHECK(std::equal(test.begin(), test.end(), scratchpad.begin()));

            const auto key = set.unpack(ids[i]);
            CHECK(key.size() == 10);
            CHECK(std::equal(test.begin(), test.end(), key.begin()));
        }
    }
}

TEST_CASE("Complex Type2 Vector")
{
    for (size_t seed = 1337; seed < (1337 + 10); ++seed) {
        auto set = ptrie::set_stable<type_t, size_t, 9>{};
        auto ids = std::vector<size_t>{};
        auto scratchpad = std::vector<type_t>(10);
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto test = type_t::rand_vec(i + seed, 10);
            const auto [res, id] = set.insert(test);
            REQUIRE(res);
            ids.push_back(id);
            const auto size = set.unpack(id, scratchpad.data());

            REQUIRE(size == 10);
            REQUIRE(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024_uz * 10; ++i) {
            const auto test = type_t::rand_vec(i + seed, 10);
            const auto size = set.unpack(ids[i], scratchpad.data());

            CHECK(size == 10);
            CHECK(std::equal(test.begin(), test.end(), scratchpad.begin()));

            const auto key = set.unpack(ids[i]);
            CHECK(key.size() == 10);
            CHECK(test == key);
        }
    }
}

TEST_CASE("Simple Copy")
{
    auto set = ptrie::set_stable<size_t>{};
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
