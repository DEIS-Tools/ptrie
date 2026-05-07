#ifndef PTRIE_INSERT_HPP
#define PTRIE_INSERT_HPP

#include "binarywrapper.h"
#include "MurmurHash2.h"

#include <random>

namespace ptrie {

inline binarywrapper_t rand_data(size_t seed, size_t maxsize, size_t minsize = sizeof(size_t), size_t mv = 256)
{
    assert(minsize >= sizeof(size_t));
    srand(seed);
    // pick size between 0 and maxsize
    size_t size = minsize != maxsize ? minsize + rand() % (maxsize - minsize) : minsize;

    auto data = binarywrapper_t(size * 8);
    // fill in random data
    for (size_t j = 0; j < size; ++j) {
        data.raw()[j] = static_cast<uchar>(rand() % mv);
    }
    // make sure everything is unique
    for (size_t j = 1; j <= sizeof(size_t); ++j) {
        data.raw()[size - j] = ((uchar*)&seed)[j - 1];
    }
    return data;
}

struct wrapper_t
{
    binarywrapper_t data;
    uint64_t _hash{};

    bool operator<(const wrapper_t& other) const
    {
        if (_hash == other._hash) {
            if (data.size() == other.data.size())
                return memcmp(data.const_raw(), other.data.const_raw(), data.size()) > 0;
            return data.size() < other.data.size();
        }
        return _hash < other._hash;
    }
};

struct hasher_o
{
    uint64_t operator()(const wrapper_t& w) const { return w._hash; }
};

struct equal_o
{
    bool operator()(const wrapper_t& w1, const wrapper_t& w2) const
    {
        if (w1._hash == w2._hash)
            return w1.data == w2.data;
        return false;
    }
};

template <typename T>
void set_insert(T& set, size_t elements, size_t seed, size_t bytes, double deletes [[maybe_unused]], double read_rate,
                size_t mv)
{
    /*
    auto generator = std::default_random_engine(seed);
    auto dist = std::uniform_real_distribution<double>{};
    auto rem = std::uniform_int_distribution<int>(0, elements);
    */

    auto read_generator = std::default_random_engine(seed);
    auto read_dist = std::normal_distribution<double>(read_rate, read_rate / 2.0);
    auto read_el = std::uniform_int_distribution<size_t>(0, elements);

    auto w = wrapper_t{};
    for (size_t i = 0; i < elements; ++i) {
        w.data = rand_data(seed + i, bytes, bytes, mv);
        w._hash = MurmurHash64A(w.data.raw(), w.data.size(), seed);
        if (w._hash == 0)
            w._hash += 1;
        else if (w._hash == std::numeric_limits<uint64_t>::max())
            w._hash -= 1;
        set.insert(w);

        if (read_rate > 0.0) {
            const int reads = std::round(read_dist(read_generator));
            for (int r = 0; r < reads; ++r) {
                const size_t el = read_el(read_generator);
                w.data = rand_data(seed + el, bytes, bytes, mv);
                w._hash = MurmurHash64A(w.data.raw(), w.data.size(), seed);
                if (w._hash == 0)
                    w._hash += 1;
                else if (w._hash == std::numeric_limits<uint64_t>::max())
                    w._hash -= 1;
                set.count(w);
                w.data.release();
            }
        }

        /*        if(dist(generator) < deletes)
                {
                    size_t el = rem(generator) % elements;
                    w.data = rand_data(seed + el, bytes, bytes, mv);
                    w._hash = MurmurHash64A(w.data.raw(), w.data.size(), seed);
                    if(w._hash == 0) w._hash += 1;
                    else if (w._hash == std::numeric_limits<uint64_t>::max())
           w._hash -= 1; auto it = set.find(w); if(it != set.end()) set.erase(it);
                    w.data.release();
                }*/
    }
    w.data = binarywrapper_t();
    for (auto& elem : set)
        const_cast<wrapper_t&>(elem).data.release();
}

template <typename T>
void set_insert_ptrie(T& set, const size_t elements, const size_t seed, const size_t bytes,
                      double deletes [[maybe_unused]], const double read_rate, const size_t mv)
{
    /*
    auto del_generator = std::default_random_engine(seed);
    auto del_dist = std::uniform_real_distribution<double>{};
    auto del_el = std::uniform_int_distribution<int>(0, elements);
    */

    auto read_generator = std::default_random_engine(seed);
    auto read_dist = std::normal_distribution<double>(read_rate, read_rate / 2.0);
    auto read_el = std::uniform_int_distribution<int>(0, elements);

    for (size_t i = 0; i < elements; ++i) {
        auto data = rand_data(seed + i, bytes, bytes, mv);
        set.insert(data.raw(), data.size());
        data.release();

        if (read_rate > 0.0) {
            int reads = std::round(read_dist(read_generator));
            for (int r = 0; r < reads; ++r) {
                size_t el = read_el(read_generator);
                data = rand_data(seed + el, bytes, bytes, mv);
                set.exists(data.raw(), data.size());
                data.release();
            }
        }
        /*if(dist(generator) < deletes)
        {
            size_t el = rem(generator) % elements;
            auto torem = rand_data(seed + el, bytes, bytes, mv);
            set.erase(torem);
            torem.release();
        }*/
    }
}
}  // namespace ptrie
#endif  // PTRIE_INSERT_HPP
