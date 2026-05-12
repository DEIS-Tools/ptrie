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
#include <map>

TEST_SUITE_BEGIN("PTrie Map");

using ptrie::uchar;

TEST_CASE("Pseudo Rand1")
{
    for (size_t seed = 314; seed < (314 + 10); ++seed) {
        auto set = ptrie::map<ptrie::uchar, size_t>{};

        for (size_t i = 0; i < 1024 * 10; ++i) {
            const auto data = rand_data(i + seed, 20);
            const auto [res, id] = set.insert(std::data(data), std::size(data));
            CHECK(res);
            set.get_data(id) = i;
        }

        // let us unwrap everything and check that it is there!

        for (size_t i = 0; i < 1024 * 10; ++i) {
            const auto data = rand_data(i + seed, 20);
            const auto [res, id] = set.exists(std::data(data), std::size(data));
            CHECK(res);
            CHECK(set.get_data(id) == i);
        }
    }
}

TEST_CASE("Pseudo Rand1 Key")
{
    constexpr auto mx = 5;
    auto data = std::vector<int32_t>(mx);
    auto unpack = std::vector<int32_t>(mx);
    for (size_t seed = 314; seed < (314 + 10); ++seed) {
        auto set = ptrie::map<int32_t, size_t>{};

        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(seed + i);
            for (auto& d : data)
                d = rand();
            const auto [res, id] = set.insert(std::data(data), mx);
            CHECK(res);
            set.get_data(id) = i;
            const auto [res2, id2] = set.exists(std::data(data), mx);
            CHECK(res2);
            CHECK(id2 == id);
        }

        // let us unwrap everything and check that it is there!

        for (size_t i = 0; i < 1024 * 10; ++i) {
            srand(seed + i);
            for (auto& d : data)
                d = rand();
            const auto [res, id] = set.exists(std::data(data), mx);
            CHECK(res);
            CHECK(set.get_data(id) == i);
            const auto size = set.unpack(id, std::data(unpack));
            CHECK(size == mx);
            CHECK(data == unpack);
        }
    }
}

TEST_CASE("Pseudo Rand Split Heap")
{
    for (size_t seed = 512; seed < (512 + 10); ++seed) {
        auto set = ptrie::map<uchar, size_t, sizeof(size_t) + 1, 6>{};
        for (size_t i = 0; i < 1024 * 10; ++i) {
            const auto data = rand_data(i + seed, 20);
            const auto [res, id] = set.insert(std::data(data), std::size(data));
            CHECK(res);
            set.get_data(id) = i;
        }

        // let us unwrap everything and check that it is there!

        for (size_t i = 0; i < 1024 * 10; ++i) {
            const auto data = rand_data(i + seed, 20);
            const auto [res, id] = set.exists(std::data(data), std::size(data));
            CHECK(res);
            const auto d = set[{std::data(data), std::size(data)}];
            CHECK(d == i);
            CHECK(set.get_data(id) == i);
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
    static type_t rand(unsigned int seed)
    {
        auto char_gen = rand_gen<char>();
        auto int_gen = rand_gen<int>(seed);
        return {char_gen(), int_gen(), char_gen(), int_gen()};
    }
    static std::vector<type_t> rand_vec(unsigned int seed, std::size_t size)
    {
        auto char_gen = rand_gen<char>();
        auto int_gen = rand_gen<int>(seed);
        auto res = std::vector<type_t>(size);
        for (auto& t : res)
            t = type_t{char_gen(), int_gen(), char_gen(), int_gen()};
        return res;
    }
    friend std::ostream& operator<<(std::ostream& os, const type_t& el)
    {
        return os << +el._a << ", " << el._b << ", " << +el._c << ", " << el._d;
    }
};

template <>
struct ptrie::byte_iterator<type_t>
{
    static constexpr uchar& access(type_t* data, size_t id)
    {
        const auto el = id / element_size();
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
        auto scratchpad = type_t{};
        for (size_t i = 0; i < 1024 * 10; ++i) {
            const auto test = type_t::rand(i + seed);
            cont[test] = i;
            CHECK(cont[test] == i);
            const auto [res, id] = cont.exists(test);
            CHECK(res);
            CHECK(i == id);
            ids.push_back(id);
            const auto size = cont.unpack(id, &scratchpad);

            CHECK(size == 1);
            CHECK(test == scratchpad);
        }

        // let us unwrap everything and check that it is there!
        for (size_t i = 0; i < 1024 * 10; ++i) {
            const auto test = type_t::rand(i + seed);
            CHECK(cont[test] == i);
            const auto size = cont.unpack(ids[i], &scratchpad);

            CHECK(size == 1);
            CHECK(test == scratchpad);

            const auto key = cont.unpack(ids[i]);
            CHECK(key.size() == 1);
            CHECK(key.back() == test);
        }
    }
}

TEST_CASE("Simple Iterator")
{
    std::cerr << "SimpleIterator" << std::endl;
    constexpr size_t x = 10000;
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

/* TODO: enable when implemented
TEST_CASE("Ranged-for-loop")
{
    auto map = ptrie::map<int,const char*>{};
    const auto* msg = "divisable by 3";
    for (auto i = -127; i < 128; ++i)
        if (i % 3 == 0)
            map[i] = msg;
    for (auto&& [key, value] : map) {
        CHECK(key % 3 == 0);
        CHECK(value = =msg);
    }
}
*/
