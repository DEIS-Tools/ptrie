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
// Created by Peter G. Jensen on 24/12/16.

#include "int_ptrie_insert.hpp"

#include "utils.h"

#include <ptrie/ptrie.h>
#include <ptrie/ptrie_map.h>

#include <sparsehash/dense_hash_set>
#include <sparsehash/sparse_hash_set>

#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>

#include <cstdlib>

int main(int argc, const char** argv)
{
    if (argc < 3 || argc > 8) {
        std::cout << "usage : <ptrie/std/sparse/dense> <number elements> <?seed> "
                     "<?delete ratio> <?read rate>"
                  << std::endl;
        exit(-1);
    }

    const char* type = argv[1];
    size_t elements = 1024;
    size_t seed = 0;
    double deletes = 0.0;
    double read_rate = 0.0;

    read_arg(argv[2], elements, "Error in <number of elements>", "%zu");
    if (argc > 3)
        read_arg(argv[3], seed, "Error in <seed>", "%zu");
    if (argc > 4)
        read_arg(argv[4], deletes, "Error in <delete ratio>", "%lf");
    if (argc > 5)
        read_arg(argv[5], read_rate, "Error in <read rate>", "%lf");

    auto order = std::vector<size_t>{};
    for (size_t i = 0; i < sizeof(size_t) * 8; ++i)
        order.push_back(i);

    std::srand(seed);
    std::random_shuffle(order.begin(), order.end());

    if (strcmp(type, "ptrie") == 0) {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = ptrie::set<>{};
        const auto sw = Timer{};
        set_insert_ptrie(set, elements, std::rand(), deletes, read_rate, order);
    } else if (strcmp(type, "ptrie-stable") == 0) {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = ptrie::set_stable<>{};
        const auto sw = Timer{};
        set_insert_ptrie(set, elements, std::rand(), deletes, read_rate, order);
    } else if (strcmp(type, "ptrie-map") == 0) {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = ptrie::map<unsigned char, size_t>{};
        const auto sw = Timer{};
        set_insert_ptrie(set, elements, std::rand(), deletes, read_rate, order);
    } else if (strcmp(type, "std") == 0) {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = std::unordered_set<size_t, hasher_o, equal_o>{};
        const auto sw = Timer{};
        set_insert(set, elements, std::rand(), deletes, read_rate, order);
    } else if (strcmp(type, "redblack") == 0) {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = std::set<size_t>{};
        const auto sw = Timer{};
        set_insert(set, elements, std::rand(), deletes, read_rate, order);
    } else if (strcmp(type, "sparse") == 0) {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = google::sparse_hash_set<size_t, hasher_o, equal_o>(elements / 10);
        const auto sw = Timer{};
        set_insert(set, elements, std::rand(), deletes, read_rate, order);
    } else if (strcmp(type, "dense") == 0) {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        seed = std::rand();
        auto set = google::dense_hash_set<size_t, hasher_o, equal_o>(elements / 10);
        const auto sw = Timer{};
        set.set_empty_key(reorder(0, order, seed));
        if (deletes > 0.0)
            set.set_deleted_key(reorder(std::numeric_limits<uint32_t>::max(), order, seed));
        set_insert(set, elements, seed, deletes, read_rate, order);
    } else {
        std::cerr << "ERROR IN TYPE, ONLY VALUES ALLOWED : ptrie, ptrie-stable, "
                     "ptrie-map, std, sparse, dense, tbb"
                  << std::endl;
        exit(-1);
    }

    return 0;
}
