#ifndef PTRIE_INSERT_HPP
#define PTRIE_INSERT_HPP

#include "utils.h"

#include "binarywrapper.h"
#include "MurmurHash2.h"

#include <random>

namespace ptrie {

inline binarywrapper_t rand_data(size_t seed, size_t maxsize, size_t minsize = sizeof(size_t), size_t mv = 256)
{
    assert(minsize >= sizeof(size_t));
    auto gen = std::default_random_engine{seed};
    auto size_dist = std::uniform_int_distribution{minsize, maxsize};
    const size_t size = size_dist(gen);

    auto data_dist = std::uniform_int_distribution<size_t>{0, mv - 1};
    auto data = binarywrapper_t{Bits{size * BYTE_BITS}};
    // fill in random data
    for (size_t j = 0; j < size; ++j)
        data.raw()[j] = static_cast<uchar>(data_dist(gen));

    // make sure everything is unique
    for (size_t j = 1; j <= sizeof(size_t); ++j) {
        data.raw()[size - j] = ((uchar*)&seed)[j - 1];  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
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
                return std::memcmp(data.const_raw(), other.data.const_raw(), data.size()) > 0;
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
void set_insert(T& set, const Settings& s)
{
    auto read_generator = std::default_random_engine{s.seed};
    auto read_dist = std::normal_distribution<double>{s.read_rate, s.read_rate / 2.0};
    auto read_el = std::uniform_int_distribution<size_t>{0, s.elements};

    auto w = wrapper_t{};
    for (size_t i = 0; i < s.elements; ++i) {
        w.data = rand_data(s.seed + i, s.bytes, s.bytes, s.maxval);
        w._hash = MurmurHash64A(w.data.raw(), w.data.size(), s.seed);
        if (w._hash == 0)
            w._hash += 1;
        else if (w._hash == std::numeric_limits<uint64_t>::max())
            w._hash -= 1;
        set.insert(w);

        if (s.read_rate > 0.0) {
            const int reads = static_cast<int>(std::round(read_dist(read_generator)));
            for (int r = 0; r < reads; ++r) {
                const size_t el = read_el(read_generator);
                w.data = rand_data(s.seed + el, s.bytes, s.bytes, s.maxval);
                w._hash = MurmurHash64A(w.data.raw(), w.data.size(), s.seed);
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
void set_insert_ptrie(T& set, const Settings& s)
{
    auto read_generator = std::default_random_engine{s.seed};
    auto read_dist = std::normal_distribution<double>{s.read_rate, s.read_rate / 2.0};
    auto read_el = std::uniform_int_distribution<size_t>{0, s.elements};

    for (size_t i = 0; i < s.elements; ++i) {
        auto data = rand_data(s.seed + i, s.bytes, s.bytes, s.maxval);
        set.insert(data.raw(), data.size());
        data.release();

        if (s.read_rate > 0.0) {
            const int reads = static_cast<int>(std::round(read_dist(read_generator)));
            for (int r = 0; r < reads; ++r) {
                const size_t el = read_el(read_generator);
                data = rand_data(s.seed + el, s.bytes, s.bytes, s.maxval);
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
