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
 * File:   ptrie.h
 * Author: Peter G. Jensen
 *
 * Created on 10 June 2015, 18:44
 */

#ifndef PTRIE_H
#define PTRIE_H

#include "ptrie_internal.hpp"
#include "ptrie_memory.hpp"

// direct 2lvl indexing in chunks of ~ 2^32
// takes up ~ 512k (256k on 32bit) for the index.
// when to keep data in bucket or send to heap - just make sure bucket does not
// overflow

namespace ptrie {

template <typename KEY = uchar, uint16_t HEAPBOUND = 17, uint16_t SPLITBOUND = 129, uint8_t BSIZE = 8,
          size_t ALLOCSIZE = (1024 * 64)>
class set : internal::ptrie_base<KEY, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, void, size_t, false>
{
    using pt = internal::ptrie_base<KEY, HEAPBOUND, SPLITBOUND, BSIZE, ALLOCSIZE, void, size_t, false>;

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

    class iterator : public internal::iterator_base<set, iterator>
    {
    public:
        using internal::iterator_base<set, iterator>::iterator_base;
    };

    iterator begin() const { return ++iterator(&this->_root, 0); }
    iterator end() const { return iterator(&this->_root, 256); }
};

}  // namespace ptrie

#endif /* PTRIE_H */
