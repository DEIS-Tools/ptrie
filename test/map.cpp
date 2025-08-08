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

#include <ptrie/ptrie_map.h>

#include <doctest/doctest.h>

#include <vector>

TEST_SUITE_BEGIN("PTrie Map");

TEST_CASE("Pseudo Rand1")
{
    for (size_t seed = 314; seed < (314 + 10); ++seed) {
        auto set = ptrie::map<ptrie::uchar, size_t>{};

        for (size_t i = 0; i < 1024 * 10; ++i) {
            auto data = rand_data(i + seed, 20);
            auto res = set.insert(std::data(data), std::size(data));
            CHECK(res.first);
            set.get_data(res.second) = i;
        }

        // let us unwrap everything and check that it is there!

        for (size_t i = 0; i < 1024 * 10; ++i) {
            auto data = rand_data(i + seed, 20);
            auto res = set.exists(std::data(data), std::size(data));
            CHECK(res.first);
            CHECK(set.get_data(res.second) == i);
        }
    }
}

TEST_CASE("Pseudo Rand1 Key")
{
    constexpr auto mx = 5;
    auto data = std::make_unique<int32_t[]>(mx);
    auto unpack = std::make_unique<int32_t[]>(mx);
    for (size_t seed = 314; seed < (314 + 10); ++seed) {
        ptrie::map<int32_t, size_t> set;

        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(seed + i);
            for (size_t j = 0; j < mx; ++j)
                data[j] = rand();
            auto res = set.insert(data.get(), mx);
            CHECK(res.first);
            set.get_data(res.second) = i;
            auto res2 = set.exists(data.get(), mx);
            CHECK(res2.first);
            CHECK(res2.second == res.second);
        }

        // let us unwrap everything and check that it is there!

        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(seed + i);
            for (size_t j = 0; j < mx; ++j)
                data[j] = rand();
            auto res = set.exists(data.get(), mx);
            CHECK(res.first);
            CHECK(set.get_data(res.second) == i);
            auto size = set.unpack(res.second, unpack.get());
            CHECK(size == mx);
            CHECK(std::equal(data.get(), data.get() + mx, unpack.get()));
        }
    }
}

TEST_CASE("Pseudo Rand Split Heap")
{
    for (size_t seed = 512; seed < (512 + 10); ++seed) {
        auto set = ptrie::map<unsigned char, size_t, sizeof(size_t) + 1, 6>{};
        for (size_t i = 0; i < 1024 * 10; ++i) {
            auto data = rand_data(i + seed, 20);
            auto res = set.insert(std::data(data), std::size(data));
            CHECK(res.first);
            set.get_data(res.second) = i;
        }

        // let us unwrap everything and check that it is there!

        for (size_t i = 0; i < 1024 * 10; ++i) {
            auto data = rand_data(i + seed, 20);
            auto res = set.exists(std::data(data), std::size(data));
            auto d = set[{std::data(data), std::size(data)}];
            CHECK(d == i);
            CHECK(res.first);
            CHECK(set.get_data(res.second) == i);
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
        assert(el == 0);
        assert(id < element_size());
        id = id % element_size();
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
        auto cont = ptrie::map<type_t, size_t>{};
        auto ids = std::vector<size_t>{};
        type_t scratchpad;
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            auto test = type_t{(char)rand(), (int)rand(), (char)rand(), (int)rand()};
            cont[test] = i;
            CHECK(cont[test] == i);
            auto res = cont.exists(test);
            CHECK(res.first);
            CHECK(i == res.second);
            ids.push_back(res.second);
            auto size = cont.unpack(res.second, &scratchpad);

            CHECK(size == 1);
            CHECK(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(i + seed);
            type_t test({(char)rand(), (int)rand(), (char)rand(), (int)rand()});
            CHECK(cont[test] == i);
            auto size = cont.unpack(ids[i], &scratchpad);

            CHECK(size == 1);
            CHECK(test == scratchpad);

            auto key = cont.unpack(ids[i]);
            CHECK(key.size() == 1);
            CHECK(key.back() == test);
        }
    }
}

TEST_CASE("Simple Iterator")
{
    std::cerr << "SimpleIterator" << std::endl;
    const size_t x = 10000;
    auto set = ptrie::map<size_t, size_t>{};
    for (size_t i = 0; i < x; ++i)
        set[i] = i;
    size_t cnt = 0;
    for (auto b = set.begin(); b != set.end(); ++b) {
        REQUIRE(b.index() == *b);
        REQUIRE(b.index() <= x);
        REQUIRE(b.index() == b.unpack().back());
        ++cnt;
    }
    CHECK(cnt == x);
}
