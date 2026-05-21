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
#include <ptrie/ptrie_memory.hpp>

#include <doctest/doctest.h>

#include <vector>
#include <algorithm>  // std::max

#include <cstddef>  // size_t

TEST_SUITE_BEGIN("PTrie Delete");

using ptrie::uchar;

const auto one_uchar_gen = [](std::size_t i) { return std::vector{static_cast<uchar>(i)}; };

TEST_CASE("Insert Delete Byte")
{
    auto set = ptrie::set_stable<>{};
    try_insert(set, one_uchar_gen, 256);
    for (int i = 255; i >= 0; --i) {
        auto data = one_uchar_gen(i);
        const bool res = set.erase(data.data(), data.size());
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);

        const auto [exists1, _] = set.exists(data.data(), data.size());
        CHECK_MESSAGE(!exists1, "FAILED ON DELETE, STILL EXISTS " << i);

        bool ok = true;
        for (int j = 0; j < 256; ++j) {
            data[0] = static_cast<uchar>(j);
            const auto [exists2, _] = set.exists(data.data(), data.size());
            if (j >= i) {
                ok &= !exists2;
                CHECK_MESSAGE(!exists2, "FAILED ON DELETE, REMOVED " << i << " REINTRODUCED " << j);
            } else {
                ok &= exists2;
                CHECK_MESSAGE(exists2, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
            }
        }
        REQUIRE(ok);
    }
}

TEST_CASE("Insert Delete Byte Mod")
{
    auto set = ptrie::set_stable<>{};
    try_insert(set, one_uchar_gen, 256);
    auto i = 256u;
    while (i-- > 0) {
        auto data = std::vector<uchar>(1);
        if (i % 2 != 0)
            data[0] = static_cast<uchar>(127 - (i / 2));
        else
            data[0] = static_cast<uchar>(128 + (i / 2));

        bool ok = true;
        const bool res = set.erase(std::data(data), 1);
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);
        ok &= res;

        const auto [exists1, _] = set.exists(std::data(data), 1);
        ok &= !exists1;
        CHECK_MESSAGE(!exists1, "FAILED ON DELETE, STILL EXISTS " << i);

        for (auto j = 0u; j < 256u; ++j) {
            if (j % 2 != 0)
                data[0] = static_cast<uchar>(127 - (j / 2));
            else
                data[0] = static_cast<uchar>(128 + (j / 2));
            const auto [exists2, _] = set.exists(std::data(data), 1);
            if (j < i) {
                CHECK_MESSAGE(exists2, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
                ok &= exists2;
            } else {
                CHECK_MESSAGE(!exists2, "FAILED ON DELETE, REMOVED " << i << " BUT REINTRODUCED " << j);
                ok &= !exists2;
            }
        }
        REQUIRE(ok);
    }
}

TEST_CASE("Insert Delete Byte Split")
{
    auto set = ptrie::set_stable<uchar, size_t, sizeof(size_t) + 1, 6>{};
    try_insert(set, one_uchar_gen, 256);
    auto i = 256u;
    while (i-- > 0) {
        auto data = one_uchar_gen(i);
        const bool res = set.erase(std::data(data), 1);
        REQUIRE_MESSAGE(res, "FAILED ON DELETE " << i);

        const auto [exists1, _] = set.exists(std::data(data), 1);
        REQUIRE_MESSAGE(!exists1, "FAILED ON DELETE, STILL EXISTS " << i);

        for (auto j = 0u; j < 256u; ++j) {
            data[0] = static_cast<uchar>(j);
            const auto [exists2, _] = set.exists(std::data(data), 1);
            if (j < i)
                REQUIRE_MESSAGE(exists2, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
            else
                REQUIRE_MESSAGE(!exists2, "FAILED ON DELETE " << i << ", REINTRODUCED " << j);

            for (auto k = 0u; k < i; ++k) {
                data[0] = static_cast<uchar>(k);
                const auto [exists3, _] = set.exists(std::data(data), 1);
                REQUIRE_MESSAGE(exists3, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
            }
        }
    }
}

TEST_CASE("Insert Delete Byte Mod Split")
{
    auto set = ptrie::set_stable<uchar, size_t, sizeof(size_t) + 1, 6>{};
    try_insert(set, one_uchar_gen, 256);
    auto i = 256u;
    while (i-- > 0) {
        auto data = one_uchar_gen(0);
        if (i % 2 != 0)
            data[0] = static_cast<uchar>(127 - (i / 2));
        else
            data[0] = static_cast<uchar>(128 + (i / 2));

        const bool res = set.erase(std::data(data), 1);
        REQUIRE_MESSAGE(res, "FAILED ON DELETE " << i);

        const auto [exists1, _] = set.exists(std::data(data), 1);
        REQUIRE_MESSAGE(!exists1, "FAILED ON DELETE, STILL EXISTS " << i);

        for (auto j = 0u; j < i; ++j) {
            if (j % 2 != 0)
                data[0] = static_cast<uchar>(127 - (j / 2));
            else
                data[0] = static_cast<uchar>(128 + (j / 2));
            const auto [exists2, _] = set.exists(std::data(data), 1);
            REQUIRE_MESSAGE(exists2, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
        }
    }
}

TEST_CASE("Insert Delete Large")
{
    constexpr auto max = 8000u;
    auto set = ptrie::set_stable<uchar, size_t, sizeof(size_t) + 1, 6>{};
    try_insert(set, [](size_t i) { return rand_data(i, 16, 16); }, max);
    auto i = max;
    while (i-- > 0) {
        auto seed = 0u;
        if (i % 2 != 0)
            seed = ((max / 2) - 1) - (i / 2);
        else
            seed = (max / 2) + (i / 2);

        const auto data = rand_data(seed, 16, 16);
        bool ok = true;
        const bool res = set.erase(std::data(data), std::size(data));
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);
        ok &= res;

        const auto [exists1, _] = set.exists(std::data(data), std::size(data));
        ok &= !exists1;
        CHECK_MESSAGE(!exists1, "FAILED ON DELETE, STILL EXISTS " << i);
        for (auto j = std::max(0u, i - 100); j < std::min(i + 100, max); ++j) {
            auto s2 = 0u;
            if (j % 2 == 1)
                s2 = ((max / 2) - 1) - (j / 2);
            else
                s2 = (max / 2) + (j / 2);
            const auto d2 = rand_data(s2, 16, 16);
            const auto [exists2, _] = set.exists(std::data(d2), std::size(d2));
            if (j < i) {
                CHECK_MESSAGE(exists2, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
                ok &= exists2;
            } else {
                CHECK_MESSAGE(!exists2, "FAILED ON DELETE, REMOVED " << i << " BUT REINTRODUCED " << j);
                ok &= !exists2;
            }
        }
        REQUIRE(ok);
    }
}

TEST_CASE("Insert Delete Large2")
{
    constexpr auto max = 8000;
    auto set = ptrie::set_stable<uchar, size_t, sizeof(size_t) + 1, 6>{};
    auto fun = [](size_t i) {
        if ((i % 2) == 0)
            return rand_data(i, 17, 15);
        return rand_data(i, 9, 8);
    };
    try_insert(set, fun, max);
    for (int i = max - 1; i >= 0; --i) {
        int seed = 0;
        if (i % 2 != 0)
            seed = ((max / 2) - 1) - (i / 2);
        else
            seed = (max / 2) + (i / 2);

        const auto data = fun(seed);
        bool ok = true;
        const bool res = set.erase(std::data(data), std::size(data));
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);
        ok &= res;

        const auto [exists1, _] = set.exists(std::data(data), std::size(data));
        ok &= !exists1;
        CHECK_MESSAGE(!exists1, "FAILED ON DELETE, STILL EXISTS " << i);
        for (int j = std::max(0, i - 100); j < std::min(i + 100, max); ++j) {
            int s2 = 0;
            if (j % 2 != 0)
                s2 = ((max / 2) - 1) - (j / 2);
            else
                s2 = (max / 2) + (j / 2);
            const auto d2 = fun(s2);
            const auto [exists2, _] = set.exists(std::data(d2), std::size(d2));
            if (j < i) {
                CHECK_MESSAGE(exists2, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
                ok &= exists2;
            } else {
                CHECK_MESSAGE(!exists2, "FAILED ON DELETE, REMOVED " << i << " BUT REINTRODUCED " << j);
                ok &= !exists2;
            }
        }
        REQUIRE(ok);
    }
}

TEST_CASE("Insert Delete Large3")
{
    constexpr auto max = 8000u;
    auto set = ptrie::set_stable<uchar, size_t, sizeof(size_t) + 1, 6>{};
    auto fun = [](size_t i) {
        if ((i % 2) == 0)
            return rand_data(i, 17, 15);
        return rand_data(i, 130, 126);
    };
    try_insert(set, fun, max);
    auto i = max;
    while (i-- > 0) {
        auto seed = 0u;
        if (i % 2 != 0)
            seed = ((max / 2) - 1) - (i / 2);
        else
            seed = (max / 2) + (i / 2);

        const auto data = fun(seed);
        bool ok = true;
        const bool res = set.erase(std::data(data), std::size(data));
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);
        ok &= res;

        const auto [exists1, _] = set.exists(std::data(data), std::size(data));
        ok &= !exists1;
        CHECK_MESSAGE(!exists1, "FAILED ON DELETE, STILL EXISTS " << i);
        for (auto j = std::max(0u, i - 100); j < std::min(i + 100, max); ++j) {
            auto s2 = 0u;
            if (j % 2 != 0)
                s2 = ((max / 2) - 1) - (j / 2);
            else
                s2 = (max / 2) + (j / 2);
            const auto d2 = fun(s2);
            const auto [exists2, _] = set.exists(std::data(d2), std::size(d2));
            if (j < i) {
                CHECK_MESSAGE(exists2, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
                ok &= exists2;
            } else {
                CHECK_MESSAGE(!exists2, "FAILED ON DELETE, REMOVED " << i << " BUT REINTRODUCED " << j);
                ok &= !exists2;
            }
        }
        REQUIRE(ok);
    }
}

TEST_SUITE_END();
