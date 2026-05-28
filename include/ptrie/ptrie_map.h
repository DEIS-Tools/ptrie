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
 * File:   ptrie_stable.h
 * Author: Peter G. Jensen
 *
 *
 * Created on 06 October 2016, 13:51
 */

#ifndef PTRIE_STABLE_H
#define PTRIE_STABLE_H

#include "ptrie_stable.h"

#include <cstddef>  // size_t
#include <cstdint>  // uint16_t

namespace ptrie {

template <typename KEY, typename T, uint16_t HEAPBOUND = 17, uint16_t SPLITBOUND = 128, uint8_t BSIZE = 8,
          std::size_t ALLOCSIZE = (1024 * 64), typename I = std::size_t>
class map : internal::set_stable_base<KEY, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, T, I>
{
    static_assert(!std::is_same_v<void, T>, "T (map-to-type) must not be void");
    using pt = internal::set_stable_base<KEY, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, T, I>;
    using entrylist_t = typename pt::entrylist_t;

public:
    using pt::erase;
    using pt::exists;
    using pt::insert;
    using pt::size;
    using pt::unpack;
    using typename pt::set_stable_base;

    using node_t = typename pt::node_t;
    using fwdnode_t = typename pt::fwdnode_t;
    using typename pt::key_t;
    static constexpr auto bsize = pt::bsize;
    static constexpr auto bdiv = pt::bdiv;
    static constexpr auto heapbound = HEAPBOUND;

    T& get_data(I index);
    const T& get_data(I index) const { return const_cast<map*>(this)->get_data(index); }

    T& operator[](KEY key) { return get_data(pt::insert(key).second); }
    T& operator[](std::pair<const KEY*, size_t> key) { return get_data(pt::insert(key.first, key.second).second); }
    T& operator[](const std::vector<KEY>& key) { return get_data(pt::insert(key.data(), key.size()).second); }

    class iterator : public internal::iterator_base<map, iterator>
    {
    public:
        iterator(const internal::base_t* base, int16_t index, entrylist_t& entries):
            internal::iterator_base<map, iterator>{base, index}, _entries{&entries}
        {}
        I index() const
        {
            return mem_load<I>(static_cast<const typename pt::node_t*>(this->_node)->entries() + this->_index);
        }
        T& operator*() const { return (*_entries)[index()]._data; }
        T& operator->() const { return (*_entries)[index()]._data; }
        /* TODO: implement proper dereferencing interface just like std::map
        std::pair<const key_t,T&> operator*() const
        {
            auto res = std::pair<const key_t,T&>{key_t{}, (*_entries)[index()]._data};
            unpack(&res.first);
            return res;
        }
        std::pair<const key_t,T&> operator->() const { return operator*(); }
        */
    private:
        entrylist_t* _entries;
    };

    friend class iterator;

    iterator begin() const { return ++iterator(&this->_root, 0, *this->_entries.get()); }
    iterator end() const { return iterator(&this->_root, 256, *this->_entries.get()); }
};

template <typename KEY, typename T, uint16_t HEAPBOUND, uint16_t SPLITBOUND, uint8_t BSIZE, size_t ALLOCSIZE,
          typename I>
T& map<KEY, T, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, I>::get_data(I index)
{
    typename pt::entry_t& ent = (*this->_entries)[index];
    return ent._data;
}
}  // namespace ptrie
#undef pt
#endif /* PTRIE_STABLE_H */
