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

#include <doctest/doctest.h>
#include <vector>

TEST_SUITE_BEGIN("PTrie Stable Set");

TEST_CASE("Simple RIterator")
{
    auto set = ptrie::set_stable<size_t>{};
    constexpr size_t x = 10000;
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
    const size_t x = 10000;
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

TEST_CASE("Pseudo Rand1")
{
    for (size_t seed = 1337; seed < (1337 + 10); ++seed) {
        auto set = ptrie::set_stable<>{};
        auto ids = std::vector<size_t>{};
        auto scratchpad = std::make_unique<unsigned char[]>(20 + sizeof(size_t));
        for (size_t i = 0; i < 1024 * 10; ++i) {
            auto data = rand_data(i + seed, 20);
            auto res = set.insert(data.first.get(), data.second);
            CHECK(res.first);
            ids.push_back(res.second);
            auto size = set.unpack(res.second, scratchpad.get());

            CHECK(data.second == size);
            CHECK(memcmp(data.first.get(), scratchpad.get(), size) == 0);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024 * 10; ++i) {
            auto data = rand_data(i + seed, 20);
            auto size = set.unpack(ids[i], scratchpad.get());

            CHECK(data.second == size);
            CHECK(memcmp(data.first.get(), scratchpad.get(), size) == 0);

            auto key = set.unpack(ids[i]);
            CHECK(data.second == key.size());
            CHECK(memcmp(data.first.get(), key.data(), size) == 0);
        }
    }
}

TEST_CASE("Pseudo Rand Split Heap")
{
    for (size_t seed = 42; seed < (42 + 10); ++seed) {
        auto set = ptrie::set_stable<unsigned char, size_t, sizeof(size_t) + 1, 6>{};
        auto ids = std::vector<size_t>{};
        auto scratchpad = std::make_unique<unsigned char[]>(20 + sizeof(size_t));

        for (size_t i = 0; i < 1024 * 10; ++i) {
            auto data = rand_data(i + seed, 20);
            auto res = set.insert(data.first.get(), data.second);
            CHECK(res.first);
            ids.push_back(res.second);
            auto size = set.unpack(res.second, scratchpad.get());

            CHECK(data.second == size);
            CHECK(memcmp(data.first.get(), scratchpad.get(), size) == 0);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024 * 10; ++i) {
            auto data = rand_data(i + seed, 20);
            auto size = set.unpack(ids[i], scratchpad.get());

            CHECK(data.second == size);
            CHECK(memcmp(data.first.get(), scratchpad.get(), size) == 0);

            auto key = set.unpack(ids[i]);
            CHECK(data.second == key.size());
            CHECK(memcmp(data.first.get(), key.data(), size) == 0);
        }
    }
}

struct type_t
{
    char _a;
    int _b;
    char _c;
    int _d;
    bool operator==(const type_t& other) const
    {
        return _a == other._a && _b == other._b && _c == other._c && _d == other._d;
    }
    friend std::ostream& operator<<(std::ostream& os, const type_t& el)
    {
        std::cerr << (int)el._a << ", " << el._b << ", " << (int)el._c << ", " << el._d;
        return os;
    }
};

template <>
struct ptrie::byte_iterator<type_t>
{
    static constexpr uchar& access(type_t* data, size_t id)
    {
        auto el = id / element_size();
        id = id % element_size();
        assert(id < element_size());

        switch (id) {
        case 0: return (uchar&)data[el]._a;
        case 1:
        case 2:
        case 3:
        case 4: return ((uchar*)&data[el]._b)[id - 1];
        case 5: return (uchar&)data[el]._c;
        default: return ((uchar*)&data[el]._d)[id - 6];
        }
    }

    static constexpr const uchar& const_access(const type_t* data, size_t id)
    {
        return access(const_cast<type_t*>(data), id);
    }

    static constexpr size_t element_size() { return sizeof(char) * 2 + sizeof(int) * 2; }

    static constexpr bool continious() { return false; }
};

TEST_CASE("Complex Type1")
{
    for (size_t seed = 1337; seed < (1337 + 10); ++seed) {
        auto set = ptrie::set_stable<type_t>{};
        auto ids = std::vector<size_t>{};
        type_t scratchpad;
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            type_t test({(char)rand(), (int)rand(), (char)rand(), (int)rand()});
            auto res = set.insert(test);
            CHECK(res.first);
            ids.push_back(res.second);
            auto size = set.unpack(res.second, &scratchpad);

            CHECK(size_t{1} == size);
            CHECK(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            type_t test({(char)rand(), (int)rand(), (char)rand(), (int)rand()});
            auto size = set.unpack(ids[i], &scratchpad);

            CHECK(size_t{1} == size);
            CHECK(test == scratchpad);

            auto key = set.unpack(ids[i]);
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
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            type_t test({(char)rand(), (int)rand(), (char)rand(), (int)rand()});
            auto res = set.insert(test);
            REQUIRE(res.first);
            ids.push_back(res.second);
            auto size = set.unpack(res.second, &scratchpad);

            REQUIRE(size == 1);
            REQUIRE(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            auto test = type_t{(char)rand(), (int)rand(), (char)rand(), (int)rand()};
            auto size = set.unpack(ids[i], &scratchpad);

            CHECK(size == 1);
            CHECK(test == scratchpad);

            auto key = set.unpack(ids[i]);
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
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            std::vector<type_t> test(10);
            for (size_t i = 0; i < 10; ++i)
                test[i] = type_t{(char)rand(), (int)rand(), (char)rand(), (int)rand()};
            auto res = set.insert(test);
            REQUIRE(res.first);
            ids.push_back(res.second);
            auto size = set.unpack(res.second, scratchpad.data());

            REQUIRE(size == 10);
            REQUIRE(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            std::vector<type_t> test(10);
            for (size_t i = 0; i < 10; ++i)
                test[i] = type_t{(char)rand(), (int)rand(), (char)rand(), (int)rand()};
            auto size = set.unpack(ids[i], scratchpad.data());

            CHECK(size == 10);
            CHECK(std::equal(test.begin(), test.end(), scratchpad.begin()));

            auto key = set.unpack(ids[i]);
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
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            auto test = std::vector<type_t>(10);
            for (size_t i = 0; i < 10; ++i)
                test[i] = type_t{(char)rand(), (int)rand(), (char)rand(), (int)rand()};
            auto res = set.insert(test);
            REQUIRE(res.first);
            ids.push_back(res.second);
            auto size = set.unpack(res.second, scratchpad.data());

            REQUIRE(size == 10);
            REQUIRE(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            auto test = std::vector<type_t>(10);
            for (size_t i = 0; i < 10; ++i)
                test[i] = type_t{(char)rand(), (int)rand(), (char)rand(), (int)rand()};
            auto size = set.unpack(ids[i], scratchpad.data());

            CHECK(size == 10);
            CHECK(std::equal(test.begin(), test.end(), scratchpad.begin()));

            auto key = set.unpack(ids[i]);
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
        const auto cpy = set;  // copy on purpose
        size_t i = 0;
        for (; i < 100000; ++i)
            REQUIRE(cpy.exists(i).first);
        for (; i < 200000; ++i)
            REQUIRE(!cpy.exists(i).first);
    }
}
