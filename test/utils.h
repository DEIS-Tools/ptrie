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

// Created by Peter G. Jensen on 12/12/16.

#ifndef PTRIE_UTILS_H
#define PTRIE_UTILS_H

#include <ptrie/ptrie_memory.hpp>

#include <doctest/doctest.h>

#include <iostream>
#include <vector>

#include <cstddef>  // size_t
#include <cstdlib>  // rand

/// Workaround for "literal suffix `uz` is a C++23 feature"
constexpr std::size_t operator""_uz(unsigned long long n) { return n; }

template <typename T>
auto rand_gen(unsigned int seed)
{
    srand(seed);                                        // NOLINT(cert-msc30-c,cert-msc50-cpp)
    return [] { return static_cast<T>(std::rand()); };  // NOLINT(cert-msc30-c,cert-msc50-cpp,concurrency-mt-unsafe)
}

template <typename T>
auto rand_gen()
{
    return [] { return static_cast<T>(std::rand()); };  // NOLINT(cert-msc30-c,cert-msc50-cpp,concurrency-mt-unsafe)
}

template <typename T, typename G>
void try_insert(T& trie, G&& generator, size_t N)
{
    for (size_t i = 0; i < N; ++i) {
        auto data = generator(i);
        auto exists = trie.exists(std::data(data), std::size(data));
        REQUIRE_MESSAGE(!exists.first, "FAILED ON INSERT " << i);

        auto inserted = trie.insert(std::data(data), std::size(data));
        REQUIRE_MESSAGE(inserted.first, "EXIST FAILED FOR " << i);
    }

    for (size_t i = 0; i < N; ++i) {
        auto data = generator(i);
        auto exists = trie.exists(std::data(data), std::size(data));
        REQUIRE_MESSAGE(exists.first, "POST EXIST CHECK FAILED FOR " << i);
    }
}

inline std::vector<unsigned char> rand_data(size_t seed, size_t maxsize, size_t minsize = sizeof(size_t))
{
    REQUIRE(minsize >= sizeof(size_t));
    auto int_gen = rand_gen<int>(seed);
    // pick size between 0 and maxsize
    const size_t size = minsize != maxsize ? minsize + (int_gen() % (maxsize - minsize)) : minsize;

    auto uchar_gen = rand_gen<unsigned char>();
    auto data = std::vector<unsigned char>(size);
    for (auto& value : data)
        value = uchar_gen();
    // make sure everything is unique
    for (size_t j = 1; j <= sizeof(size_t); ++j) {
        data[size - j] = ptrie::as_array(&seed)[j - 1];
    }
    return data;
}

/// Pretty prints the container content when test fails
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    const auto end = v.end();
    os << '[';
    if (auto it = v.begin(); it != end) {
        os << *it;
        while (++it != end)
            os << ", " << *it;
    }
    return os << ']';
}

#endif  // PTRIE_UTILS_H
