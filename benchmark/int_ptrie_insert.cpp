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

#include <ptrie/ptrie_map.h>
#include <ptrie/ptrie_stable.h>
#include <ptrie/ptrie.h>

#include <sparsehash/dense_hash_set>
#include <sparsehash/sparse_hash_set>

#include <iostream>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>
#include <limits>
#include <numeric>    // iota
#include <algorithm>  // shuffle
#include <exception>

#include <cstdlib>  // rand
#include <cstdint>  // uint32_t

int main(int argc, const char** argv)
try {
    if (argc < 3 || argc > 8) {
        std::cerr << "Wrong number of arguments, expected 2-8\n";
        std::cout << "Usage: (ptrie|std|sparse|dense) (number elements) ?(seed) ?(delete ratio) ?(read rate)\n";
        return 1;
    }
    auto settings = cli_settings(argc, argv);
    auto order = std::vector<size_t>(sizeof(size_t) * 8);
    std::iota(order.begin(), order.end(), 0);

    std::srand(settings.seed);
    auto gen = std::default_random_engine(settings.seed);
    std::shuffle(order.begin(), order.end(), gen);

    std::cout << settings;
    if (settings.type == "ptrie") {
        auto set = ptrie::set<>{};
        const auto sw = Timer{};
        set_insert_ptrie(set, settings, order);
    } else if (settings.type == "ptrie-stable") {
        auto set = ptrie::set_stable<>{};
        const auto sw = Timer{};
        set_insert_ptrie(set, settings, order);
    } else if (settings.type == "ptrie-map") {
        auto set = ptrie::map<unsigned char, size_t>{};
        const auto sw = Timer{};
        set_insert_ptrie(set, settings, order);
    } else if (settings.type == "std") {
        auto set = std::unordered_set<size_t, hasher_o, equal_o>{};
        const auto sw = Timer{};
        set_insert(set, settings, order);
    } else if (settings.type == "redblack") {
        auto set = std::set<size_t>{};
        const auto sw = Timer{};
        set_insert(set, settings, order);
    } else if (settings.type == "sparse") {
        auto set = google::sparse_hash_set<size_t, hasher_o, equal_o>(settings.elements / 10);
        const auto sw = Timer{};
        set_insert(set, settings, order);
    } else if (settings.type == "dense") {
        settings.seed = std::random_device{}();
        auto set = google::dense_hash_set<size_t, hasher_o, equal_o>(settings.elements / 10);
        const auto sw = Timer{};
        set.set_empty_key(reorder(0, order, settings.seed));
        if (settings.deletes > 0.0)
            set.set_deleted_key(reorder(std::numeric_limits<uint32_t>::max(), order, settings.seed));
        set_insert(set, settings, order);
    } else
        throw std::invalid_argument{
            "ERROR IN TYPE, ALLOWED VALUES: ptrie, ptrie-stable, ptrie-map, std, sparse, dense, tbb"};
    return 0;
} catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
} catch (...) {
    std::cerr << "Caught unknown exception\n";
    return 1;
}
