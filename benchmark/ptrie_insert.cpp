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
#include <random>
#include <unordered_set>

using ptrie::binarywrapper_t;
using ptrie::uchar;
using ptrie::wrapper_t;
using ptrie::hasher_o;
using ptrie::equal_o;

int main(int argc, const char** argv)
try {
    if (argc < 3 || argc > 8) {
        std::cerr << "Wrong number of arguments, expected 2-7" << std::endl;
        std::cout << "Usage: (ptrie|std|sparse|dense) (number elements) ?(seed) "
                     "?(number of bytes) ?(delete ratio) ?(read rate) ?(max byte val)"
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }
    auto s = cli_settings(argc, argv);
    std::cout << s;
    if (s.type == "ptrie") {
        auto set = ptrie::set<>{};
        const auto sw = Timer{};
        ptrie::set_insert_ptrie(set, s);
    } else if (s.type == "std") {
        auto set = std::unordered_set<wrapper_t, hasher_o, equal_o>{};
        const auto sw = Timer{};
        set_insert(set, s);
    } else if (s.type == "sparse") {
        auto set = google::sparse_hash_set<wrapper_t, hasher_o, equal_o>(s.elements / 10);
        const auto sw = Timer{};
        ptrie::set_insert(set, s);
    } else if (s.type == "dense") {
        auto set = google::dense_hash_set<wrapper_t, hasher_o, equal_o>(s.elements / 10);
        const auto sw = Timer{};
        auto empty = wrapper_t{.data{}, ._hash = 0};
        auto del = wrapper_t{.data{}, ._hash = std::numeric_limits<uint64_t>::max()};
        set.set_empty_key(empty);
        if (s.deletes > 0.0)
            set.set_deleted_key(del);
        ptrie::set_insert(set, s);
    } else
        throw std::logic_error{"ERROR IN TYPE, ONLY VALUES ALLOWED: ptrie, std, sparse, dense"};
    return EXIT_SUCCESS;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "Caught unknown exception!" << std::endl;
    return EXIT_FAILURE;
}
