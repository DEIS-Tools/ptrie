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

/*
 * File:   ptrie_map.h
 * Author: Peter G. Jensen
 *
 *
 * Created on 06 October 2016, 13:51
 */

#ifndef PTRIE_MAP_H
#define PTRIE_MAP_H

#include "ptrie_internal.hpp"

namespace ptrie {

namespace internal {
#define SPTRIETPL \
    typename KEY, uint16_t HEAPBOUND, uint16_t SPLITBOUND, uint8_t BSIZE, size_t ALLOCSIZE, typename T, typename I
#define SPTRIETPLA KEY, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, T, I
template <typename KEY = unsigned char, uint16_t HEAPBOUND = 17, uint16_t SPLITBOUND = 128, uint8_t BSIZE = 8,
          size_t ALLOCSIZE = (1024 * 64), typename T = void, typename I = size_t>
class set_stable_base : protected ptrie_base<KEY, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, T, I, true>
{
    using pt = ptrie_base<KEY, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, T, I, true>;
    static_assert(std::is_integral_v<I>, "I (index-type) must be an integral");

public:
    using pt::erase;
    using pt::exists;
    using pt::insert;
    using typename pt::ptrie_base;

    using node_t = typename pt::node_t;
    using fwdnode_t = typename pt::fwdnode_t;
    using typename pt::key_t;

    static constexpr auto bsize = pt::bsize;
    static constexpr auto bdiv = pt::bdiv;
    static constexpr auto heapbound = HEAPBOUND;

    size_t size() const { return this->_entries->size(); }

    size_t unpack(I index, KEY* destination) const;
    std::vector<KEY> unpack(I index) const;
    void unpack(I index, std::vector<KEY>& destination) const;

    class siterator : public iterator_base<set_stable_base, siterator>
    {
    public:
        siterator(const base_t* base, int16_t index): iterator_base<set_stable_base, siterator>(base, index) {}
        I index() const { return mem_load<I>(static_cast<const node_t*>(this->_node)->entries() + this->_index); }
    };

    friend class siterator;

protected:
    node_t* find_metadata(I index, size_t& bindex) const;
    using entrylist_t = typename pt::entrylist_t;
};

template <SPTRIETPL>
typename set_stable_base<SPTRIETPLA>::node_t* set_stable_base<SPTRIETPLA>::find_metadata(I index, size_t& bindex) const
{
    node_t* node = nullptr;  // NOLINT(misc-const-correctness)
    // we can find size without bothering anyone (too much)

    bindex = 0;
    {
#ifndef NDEBUG
        bool found = false;
#endif
        const auto& entry = (*this->_entries)[index];
        node = entry._node;
        const auto& bucket = node->_data;
        const I* ents = bucket.entries(node->_count);
        for (size_t i = 0; i < node->_count; ++i) {
            if (mem_load<size_t>(ents + i) == index) {
                bindex = i;
#ifndef NDEBUG
                found = true;
#endif
                break;
            }
        }
        assert(found);
    }
    return node;
}

template <SPTRIETPL>
size_t set_stable_base<SPTRIETPLA>::unpack(I index, KEY* dest) const
{
    size_t bindex;
    auto node = find_metadata(index, bindex);
    return siterator(node, bindex).unpack(dest);
}

template <SPTRIETPL>
std::vector<KEY> set_stable_base<SPTRIETPLA>::unpack(I index) const
{
    size_t bindex;
    auto node = find_metadata(index, bindex);
    return siterator(node, bindex).unpack();
}

template <SPTRIETPL>
void set_stable_base<SPTRIETPLA>::unpack(I index, std::vector<KEY>& dest) const
{
    size_t bindex;
    auto node = find_metadata(index, bindex);
    return siterator(node, bindex).unpack(dest);
}
}  // namespace internal

template <typename KEY = unsigned char, typename I = size_t, uint16_t HEAPBOUND = 17, uint16_t SPLITBOUND = 128,
          uint8_t BSIZE = 8, size_t ALLOCSIZE = (1024 * 64)>
class set_stable : internal::set_stable_base<KEY, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, void, I>
{
    using pt = internal::set_stable_base<KEY, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, void, I>;
    using iterator = typename pt::siterator;

public:
    using pt::erase;
    using pt::exists;
    using pt::insert;
    using pt::size;
    using pt::unpack;
    using typename pt::ptrie_base;

    iterator begin() const { return ++iterator(&this->_root, 0); }
    iterator end() const { return iterator(&this->_root, 256); }
};
}  // namespace ptrie

#undef pt
#endif /* PTRIE_MAP_H */
