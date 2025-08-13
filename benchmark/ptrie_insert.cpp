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

#include "ptrie_insert.hpp"
#include "utils.h"

#include <ptrie/ptrie.h>

#include <sparsehash/dense_hash_set>
#include <sparsehash/sparse_hash_set>

#include <iostream>

#include <chrono>
#include <random>
#include <unordered_set>

#include <cstdlib>

using ptrie::binarywrapper_t;
using ptrie::uchar;
using ptrie::wrapper_t;
using ptrie::hasher_o;
using ptrie::equal_o;

int main(int argc, const char** argv)
{
    if (argc < 3 || argc > 8) {
        std::cout << "usage : <ptrie/std/sparse/dense> <number elements> <?seed> "
                     "<?number of bytes> <?delete ratio> <?read rate> <?max byte val>"
                  << std::endl;
        exit(-1);
    }

    const char* type = argv[1];
    size_t elements = 1024;
    size_t seed = 0;
    size_t bytes = 16;
    double deletes = 0.0;
    double read_rate = 0.0;
    size_t maxval = 256;

    read_arg<size_t>(argv[2], elements, "Error in <number of elements>", "%zu");
    if (argc > 3)
        read_arg<size_t>(argv[3], seed, "Error in <seed>", "%zu");
    if (argc > 4)
        read_arg<size_t>(argv[4], bytes, "Error in <bytes>", "%zu");
    if (argc > 5)
        read_arg<double>(argv[5], deletes, "Error in <delete ratio>", "%lf");
    if (argc > 6)
        read_arg<double>(argv[6], read_rate, "Error in <read rate>", "%lf");
    if (argc > 7)
        read_arg<size_t>(argv[7], maxval, "Error in <max byte val>", "%zu");

    if (strcmp(type, "ptrie") == 0) {
        print_settings(type, elements, seed, bytes, deletes, read_rate, maxval);
        auto set = ptrie::set<>{};
        const auto sw = Timer{};
        ptrie::set_insert_ptrie(set, elements, seed, bytes, deletes, read_rate, maxval);
    } else if (strcmp(type, "std") == 0) {
        print_settings(type, elements, seed, bytes, deletes, read_rate, maxval);
        auto set = std::unordered_set<wrapper_t, hasher_o, equal_o>{};
        const auto sw = Timer{};
        set_insert(set, elements, seed, bytes, deletes, read_rate, maxval);
    } else if (strcmp(type, "sparse") == 0) {
        print_settings(type, elements, seed, bytes, deletes, read_rate, maxval);
        auto set = google::sparse_hash_set<wrapper_t, hasher_o, equal_o>(elements / 10);
        const auto sw = Timer{};
        ptrie::set_insert(set, elements, seed, bytes, deletes, read_rate, maxval);
    } else if (strcmp(type, "dense") == 0) {
        print_settings(type, elements, seed, bytes, deletes, read_rate, maxval);
        auto set = google::dense_hash_set<wrapper_t, hasher_o, equal_o>(elements / 10);
        const auto sw = Timer{};
        auto empty = wrapper_t{.data{}, ._hash = 0};
        auto del = wrapper_t{.data{}, ._hash = std::numeric_limits<uint64_t>::max()};
        set.set_empty_key(empty);
        if (deletes > 0.0)
            set.set_deleted_key(del);
        ptrie::set_insert(set, elements, seed, bytes, deletes, read_rate, maxval);
    } else {
        std::cerr << "ERROR IN TYPE, ONLY VALUES ALLOWED : ptrie, std, sparse, dense" << std::endl;
        exit(-1);
    }

    return 0;
}
