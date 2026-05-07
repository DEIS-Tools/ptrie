#ifndef INT_PTRIE_INSERT_HPP
#define INT_PTRIE_INSERT_HPP

#include "binarywrapper.h"
#include "MurmurHash2.h"

#include <random>
#include <vector>

inline size_t reorder(size_t el, const std::vector<size_t>& order, size_t seed)
{
    el = el ^ seed;
    auto s = ptrie::binarywrapper_t{(ptrie::uchar*)&el, sizeof(size_t) * 8};
    auto t = ptrie::binarywrapper_t{sizeof(size_t) * 8};

    size_t& target = *(size_t*)t.raw();
    bool flip[8];
    for (size_t i = 0; i < 8; ++i)
        flip[i] = s.at(i);
    for (size_t i = 0; i < sizeof(size_t) * 8; ++i) {
        if (i <= 7) {
            t.set(order[i], s.at(i));
        } else {
            t.set(order[i], flip[i % 8] xor s.at(i));
        }
        for (auto&& f : flip)
            f = f xor t.at(order[i]);
    }
    return target;
}

template <typename T>
void set_insert(T& set, size_t elements, size_t seed, double deletes, double read_rate, std::vector<size_t>& order)
{
    auto generator = std::default_random_engine(seed);
    auto dist = std::uniform_real_distribution<double>{};

    auto read_generator = std::default_random_engine(seed);
    auto read_dist = std::normal_distribution<double>(read_rate, read_rate / 2.0);
    auto read_el = std::uniform_int_distribution<size_t>(1, elements);

    for (size_t i = 1; i <= elements; ++i) {
        size_t val = reorder(i, order, seed);
        set.insert(val);

        if (read_rate > 0.0) {
            int reads = static_cast<int>(std::round(read_dist(read_generator)));
            for (int r = 0; r < reads; ++r) {
                size_t el = reorder(read_el(read_generator), order, seed);
                set.count(el);
            }
        }

        if (dist(generator) < deletes) {
            auto rem = std::uniform_int_distribution<size_t>{1, i};
            size_t el = reorder(rem(generator), order, seed);
            auto it = set.find(el);
            if (it != set.end())
                set.erase(it);
        }
    }
}

template <typename T>
void set_insert_ptrie(T& set, size_t elements, size_t seed, double deletes, double read_rate,
                      std::vector<size_t>& order)
{
    auto del_generator = std::default_random_engine(seed);
    auto del_dist = std::uniform_real_distribution<double>{};

    auto read_generator = std::default_random_engine(seed);
    auto read_dist = std::normal_distribution<double>(read_rate, read_rate / 2.0);
    auto read_el = std::uniform_int_distribution<size_t>(1, elements);

    for (size_t i = 1; i <= elements; ++i) {
        size_t val = reorder(i, order, seed);
        set.insert((unsigned char*)&val, sizeof(val));

        if (read_rate > 0.0) {
            auto reads = static_cast<int>(std::round(read_dist(read_generator)));
            for (int r = 0; r < reads; ++r) {
                size_t el = reorder(read_el(read_generator), order, seed);
                set.exists((unsigned char*)&el, sizeof(el));
            }
        }
        if (del_dist(del_generator) < deletes) {
            std::uniform_int_distribution<size_t> del_el(1, i);
            size_t el = reorder(del_el(del_generator), order, seed);
            set.erase((unsigned char*)&el, sizeof(el));
        }
    }
}

struct hasher_o
{
    uint64_t operator()(const size_t& w) const { return MurmurHash64A(&w, sizeof(size_t), 0); }
};

struct equal_o
{
    bool operator()(const size_t& w1, const size_t& w2) const { return w1 == w2; }
};

#endif  // INT_PTRIE_INSERT_HPP
