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
try {
    if (argc < 3 || argc > 8) {
        std::cerr << "Wrong number of arguments, expected 2-7" << std::endl;
        std::cout << "Usage: (ptrie|std|sparse|dense) (number elements) ?(seed) "
                     "?(number of bytes) ?(delete ratio) ?(read rate) ?(max byte val)"
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto type = std::string_view{argv[1]};
    auto elements = read_arg(size_t{1024}, argv[2], "<number of elements>");
    size_t seed = 0;
    size_t bytes = 16;
    auto deletes = 0.0;
    auto read_rate = 2.0;
    size_t maxval = 256;
    if (argc > 3)
        seed = read_arg(seed, argv[3], "<seed>");
    if (argc > 4)
        bytes = read_arg(bytes, argv[4], "<bytes>");
    if (argc > 5)
        deletes = read_arg(deletes, argv[5], "<delete ratio>");
    if (argc > 6) {
        read_rate = read_arg(read_rate, argv[6], "<read rate>");
        if (read_rate <= 0)
            throw std::invalid_argument{"<read rate> must be greater than 0"};
    }
    if (argc > 7)
        maxval = read_arg(maxval, argv[7], "<max byte val>");

    if (type == "ptrie") {
        print_settings(type, elements, seed, bytes, deletes, read_rate, maxval);
        auto set = ptrie::set<>{};
        const auto sw = Timer{};
        ptrie::set_insert_ptrie(set, elements, seed, bytes, deletes, read_rate, maxval);
    } else if (type == "std") {
        print_settings(type, elements, seed, bytes, deletes, read_rate, maxval);
        auto set = std::unordered_set<wrapper_t, hasher_o, equal_o>{};
        const auto sw = Timer{};
        set_insert(set, elements, seed, bytes, deletes, read_rate, maxval);
    } else if (type == "sparse") {
        print_settings(type, elements, seed, bytes, deletes, read_rate, maxval);
        auto set = google::sparse_hash_set<wrapper_t, hasher_o, equal_o>(elements / 10);
        const auto sw = Timer{};
        ptrie::set_insert(set, elements, seed, bytes, deletes, read_rate, maxval);
    } else if (type == "dense") {
        print_settings(type, elements, seed, bytes, deletes, read_rate, maxval);
        auto set = google::dense_hash_set<wrapper_t, hasher_o, equal_o>(elements / 10);
        const auto sw = Timer{};
        auto empty = wrapper_t{.data{}, ._hash = 0};
        auto del = wrapper_t{.data{}, ._hash = std::numeric_limits<uint64_t>::max()};
        set.set_empty_key(empty);
        if (deletes > 0.0)
            set.set_deleted_key(del);
        ptrie::set_insert(set, elements, seed, bytes, deletes, read_rate, maxval);
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
