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
try {
    if (argc < 3 || argc > 8) {
        std::cerr << "Wrong number of arguments, expected 2-8" << std::endl;
        std::cout << "Usage: (ptrie|std|sparse|dense) (number elements) ?(seed) ?(delete ratio) ?(read rate)"
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }
    auto type = std::string_view{argv[1]};
    size_t seed = 0;
    double deletes = 0.0;
    double read_rate = 2.0;
    auto elements = read_arg(size_t{1024}, argv[2], "<number of elements>");
    if (argc > 3)
        seed = read_arg(seed, argv[3], "<seed>");
    if (argc > 4)
        deletes = read_arg(deletes, argv[4], "<delete ratio>");
    if (argc > 5) {
        read_rate = read_arg(read_rate, argv[5], "<read rate>");
        if (read_rate <= 0.0)
            throw std::invalid_argument{"read rate must be greater than zero"};
    }

    auto order = std::vector<size_t>{};
    for (size_t i = 0; i < sizeof(size_t) * 8; ++i)
        order.push_back(i);

    std::srand(seed);
    auto gen = std::default_random_engine(seed);
    std::shuffle(order.begin(), order.end(), gen);

    if (type == "ptrie") {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = ptrie::set<>{};
        const auto sw = Timer{};
        set_insert_ptrie(set, elements, std::rand(), deletes, read_rate, order);
    } else if (type == "ptrie-stable") {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = ptrie::set_stable<>{};
        const auto sw = Timer{};
        set_insert_ptrie(set, elements, std::rand(), deletes, read_rate, order);
    } else if (type == "ptrie-map") {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = ptrie::map<unsigned char, size_t>{};
        const auto sw = Timer{};
        set_insert_ptrie(set, elements, std::rand(), deletes, read_rate, order);
    } else if (type == "std") {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = std::unordered_set<size_t, hasher_o, equal_o>{};
        const auto sw = Timer{};
        set_insert(set, elements, std::rand(), deletes, read_rate, order);
    } else if (type == "redblack") {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = std::set<size_t>{};
        const auto sw = Timer{};
        set_insert(set, elements, std::rand(), deletes, read_rate, order);
    } else if (type == "sparse") {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        auto set = google::sparse_hash_set<size_t, hasher_o, equal_o>(elements / 10);
        const auto sw = Timer{};
        set_insert(set, elements, std::rand(), deletes, read_rate, order);
    } else if (type == "dense") {
        print_settings(type, elements, seed, sizeof(size_t), deletes, read_rate, 256);
        seed = std::rand();
        auto set = google::dense_hash_set<size_t, hasher_o, equal_o>(elements / 10);
        const auto sw = Timer{};
        set.set_empty_key(reorder(0, order, seed));
        if (deletes > 0.0)
            set.set_deleted_key(reorder(std::numeric_limits<uint32_t>::max(), order, seed));
        set_insert(set, elements, seed, deletes, read_rate, order);
    } else
        throw std::invalid_argument{
            "ERROR IN TYPE, ALLOWED VALUES: ptrie, ptrie-stable, ptrie-map, std, sparse, dense, tbb"};
    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
} catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
}
