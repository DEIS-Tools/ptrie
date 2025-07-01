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
#include <doctest/doctest.h>

#include <ptrie/ptrie_stable.h>
#include <vector>
#include "utils.h"

TEST_SUITE_BEGIN("PTrie Delete");

using ptrie::uchar;

TEST_CASE("Insert Delete Byte")
{
    auto set = ptrie::set_stable<>{};
    try_insert(
        set,
        [](size_t i) {
            auto data = std::make_unique<uchar[]>(1);
            data[0] = (uchar)i;
            return std::make_pair(std::move(data), 1);
        },
        256);
    for (int i = 255; i >= 0; --i) {
        auto data = std::vector{static_cast<uchar>(i)};
        bool res = set.erase(data.data(), data.size());
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);

        auto exists = set.exists(data.data(), data.size());
        CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, STILL EXISTS " << i);

        bool ok = true;
        for (int j = 0; j < 256; ++j) {
            data[0] = (uchar)j;
            auto exists = set.exists(data.data(), data.size());
            if (j >= i) {
                ok &= !exists.first;
                CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, REMOVED " << i << " REINTRODUCED " << j);
            } else {
                ok &= exists.first;
                CHECK_MESSAGE(exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
            }
        }
        REQUIRE(ok);
    }
}

TEST_CASE("Insert Delete Byte Mod")
{
    auto set = ptrie::set_stable<>{};
    try_insert(
        set,
        [](size_t i) {
            auto data = std::make_unique<unsigned char[]>(1);
            data[0] = (uchar)i;
            return std::make_pair(std::move(data), 1);
        },
        256);
    for (int i = 255; i >= 0; --i) {
        auto data = std::make_unique<unsigned char[]>(8);
        if (i % 2)
            data[0] = (uchar)(127 - (i / 2));
        else
            data[0] = (uchar)(128 + (i / 2));

        bool ok = true;
        bool res = set.erase(data.get(), 1);
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);
        ok &= res;

        auto exists = set.exists(data.get(), 1);
        ok &= !exists.first;
        CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, STILL EXISTS " << i);

        for (int j = 0; j < 256; ++j) {
            if (j % 2)
                data[0] = (uchar)(127 - (j / 2));
            else
                data[0] = (uchar)(128 + (j / 2));
            auto exists = set.exists(data.get(), 1);
            if (j < i) {
                CHECK_MESSAGE(exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
                ok &= exists.first;
            } else {
                CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT REINTRODUCED " << j);
                ok &= !exists.first;
            }
        }
        REQUIRE(ok);
    }
}

TEST_CASE("Insert Delete Byte Split")
{
    auto set = ptrie::set_stable<uchar, size_t, sizeof(size_t) + 1, 6>{};
    try_insert(
        set,
        [](size_t i) {
            auto data = std::make_unique<uchar[]>(1);
            data[0] = (uchar)i;
            return std::make_pair(std::move(data), 1);
        },
        256);
    for (int i = 255; i >= 0; --i) {
        auto data = std::make_unique<uchar[]>(1);
        data[0] = (uchar)i;
        bool res = set.erase(data.get(), 1);
        REQUIRE_MESSAGE(res, "FAILED ON DELETE " << i);

        auto exists = set.exists(data.get(), 1);
        REQUIRE_MESSAGE(!exists.first, "FAILED ON DELETE, STILL EXISTS " << i);

        for (int j = 0; j < 256; ++j) {
            data[0] = j;
            auto exists = set.exists(data.get(), 1);
            if (j < i)
                REQUIRE_MESSAGE(exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
            else
                REQUIRE_MESSAGE(!exists.first, "FAILED ON DELETE " << i << ", REINTRODUCED " << j);

            for (int j = 0; j < i; ++j) {
                data[0] = (uchar)j;
                auto exists = set.exists(data.get(), 1);
                REQUIRE_MESSAGE(exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
            }
        }
    }
}

TEST_CASE("Insert Delete Byte Mod Split")
{
    auto set = ptrie::set_stable<unsigned char, size_t, sizeof(size_t) + 1, 6>{};
    try_insert(
        set,
        [](size_t i) {
            auto data = std::make_unique<unsigned char[]>(1);
            data[0] = (uchar)i;
            return std::make_pair(std::move(data), 1);
        },
        256);
    for (int i = 255; i >= 0; --i) {
        auto data = std::make_unique<unsigned char[]>(1);
        if (i % 2)
            data[0] = (uchar)127 - (i / 2);
        else
            data[0] = (uchar)128 + (i / 2);

        bool res = set.erase(data.get(), 1);
        REQUIRE_MESSAGE(res, "FAILED ON DELETE " << i);

        auto exists = set.exists(data.get(), 1);
        REQUIRE_MESSAGE(!exists.first, "FAILED ON DELETE, STILL EXISTS " << i);

        for (int j = 0; j < i; ++j) {
            if (j % 2)
                data[0] = (uchar)127 - (j / 2);
            else
                data[0] = (uchar)128 + (j / 2);
            auto exists = set.exists(data.get(), 1);
            REQUIRE_MESSAGE(exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
        }
    }
}

TEST_CASE("Insert Delete Large")
{
    const int max = 8000;
    auto set = ptrie::set_stable<unsigned char, size_t, sizeof(size_t) + 1, 6>{};
    try_insert(set, [](size_t i) { return rand_data(i, 16, 16); }, max);
    for (int i = max - 1; i >= 0; --i) {
        int seed = 0;
        if (i % 2)
            seed = ((max / 2) - 1) - (i / 2);
        else
            seed = (max / 2) + (i / 2);

        auto data = rand_data(seed, 16, 16);
        bool ok = true;
        bool res = set.erase(data.first.get(), data.second);
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);
        ok &= res;

        auto exists = set.exists(data.first.get(), data.second);
        ok &= !exists.first;
        CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, STILL EXISTS " << i);
        for (int j = std::max(0, i - 100); j < std::min(i + 100, max); ++j) {
            int s2 = 0;
            if (j % 2)
                s2 = ((max / 2) - 1) - (j / 2);
            else
                s2 = (max / 2) + (j / 2);
            auto d2 = rand_data(s2, 16, 16);
            auto exists = set.exists(d2.first.get(), d2.second);
            if (j < i) {
                CHECK_MESSAGE(exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
                ok &= exists.first;
            } else {
                CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT REINTRODUCED " << j);
                ok &= !exists.first;
            }
        }
        REQUIRE(ok);
    }
}

TEST_CASE("Insert Delete Large2")
{
    const int max = 8000;
    auto set = ptrie::set_stable<unsigned char, size_t, sizeof(size_t) + 1, 6>{};
    auto fun = [](size_t i) {
        if ((i % 2) == 0)
            return rand_data(i, 17, 15);
        return rand_data(i, 9, 8);
    };
    try_insert(set, fun, max);
    for (int i = max - 1; i >= 0; --i) {
        int seed = 0;
        if (i % 2)
            seed = ((max / 2) - 1) - (i / 2);
        else
            seed = (max / 2) + (i / 2);

        auto data = fun(seed);
        bool ok = true;
        bool res = set.erase(data.first.get(), data.second);
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);
        ok &= res;

        auto exists = set.exists(data.first.get(), data.second);
        ok &= !exists.first;
        CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, STILL EXISTS " << i);
        for (int j = std::max(0, i - 100); j < std::min(i + 100, max); ++j) {
            int s2 = 0;
            if (j % 2)
                s2 = ((max / 2) - 1) - (j / 2);
            else
                s2 = (max / 2) + (j / 2);
            auto d2 = fun(s2);
            auto exists = set.exists(d2.first.get(), d2.second);
            if (j < i) {
                CHECK_MESSAGE(exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
                ok &= exists.first;
            } else {
                CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT REINTRODUCED " << j);
                ok &= !exists.first;
            }
        }
        REQUIRE(ok);
    }
}

TEST_CASE("Insert Delete Large3")
{
    const int max = 8000;
    auto set = ptrie::set_stable<unsigned char, size_t, sizeof(size_t) + 1, 6>{};
    auto fun = [](size_t i) {
        if ((i % 2) == 0)
            return rand_data(i, 17, 15);
        return rand_data(i, 130, 126);
    };
    try_insert(set, fun, max);
    for (int i = max - 1; i >= 0; --i) {
        int seed = 0;
        if (i % 2)
            seed = ((max / 2) - 1) - (i / 2);
        else
            seed = (max / 2) + (i / 2);

        auto data = fun(seed);
        bool ok = true;
        bool res = set.erase(data.first.get(), data.second);
        CHECK_MESSAGE(res, "FAILED ON DELETE " << i);
        ok &= res;

        auto exists = set.exists(data.first.get(), data.second);
        ok &= !exists.first;
        CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, STILL EXISTS " << i);
        for (int j = std::max(0, i - 100); j < std::min(i + 100, max); ++j) {
            int s2 = 0;
            if (j % 2)
                s2 = ((max / 2) - 1) - (j / 2);
            else
                s2 = (max / 2) + (j / 2);
            auto d2 = fun(s2);
            auto exists = set.exists(d2.first.get(), d2.second);
            if (j < i) {
                CHECK_MESSAGE(exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT ALSO DELETED " << j);
                ok &= exists.first;
            } else {
                CHECK_MESSAGE(!exists.first, "FAILED ON DELETE, REMOVED " << i << " BUT REINTRODUCED " << j);
                ok &= !exists.first;
            }
        }
        REQUIRE(ok);
    }
}

TEST_SUITE_END();