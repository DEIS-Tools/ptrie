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
 * File:   binarywrapper.h
 * Author: Peter G. Jensen
 *
 * Created on 10 June 2015, 19:20
 */

#ifndef BINARYWRAPPER_H
#define BINARYWRAPPER_H

#include <algorithm>  // std::min

#include <cassert>
#include <cstdint>
#include <cstdlib>  // calloc, free
#include <cstring>  // memcmp

namespace ptrie {
constexpr auto PTR_SIZE = sizeof(uintptr_t);  // SIZE OF POINTER!
using uint = unsigned int;
using uchar = unsigned char;
constexpr uchar Bx80 = 0x80;

/**
 * Wrapper for binary data. This provides easy access to individual bits,
 * heap allocation and comparison. Notice that one has to make sure to
 * explicitly call release() if one wishes to deallocate (possibly shared data).
 */
struct binarywrapper_t
{
    /// Default constructor does not allocate any data
    binarywrapper_t() = default;

    /// Allocates a room for at least size bits
    explicit binarywrapper_t(uint size);

    /**
     * Assign (not copy) raw data to pointer. Set number of bytes to size
     * @param raw: some memory to point to
     * @param size: number of bytes.
     */
    binarywrapper_t(uchar* raw, uint size);

    /**
     * number of bytes allocated in heap
     * @return
     */
    uint size() const { return _nbytes; }

    /**
     * Raw access to data
     * @return
     */
    uchar* raw()
    {
        if (_nbytes <= PTR_SIZE)
            return offset((uchar*)&_blob, _nbytes);
        return offset(_blob, _nbytes);
    }

    /**
     * Raw access to data when in const setting
     * @return
     */
    uchar* const_raw() const { return const_cast<binarywrapper_t*>(this)->raw(); }

    /**
     * Change value of place'th bit
     * @param place: index of bit to change
     * @param value: desired value
     */
    void set(const uint place, const bool value)
    {
        assert(place < _nbytes * 8);
        uint offset = place % 8;
        uint theplace = place / 8;
        if (value) {
            raw()[theplace] |= (Bx80 >> offset);
        } else {
            raw()[theplace] &= ~(Bx80 >> offset);
        }
    }

    /**
     * Get value of the place'th bit
     * @param place: bit index
     * @return
     */
    bool at(const uint place) const
    {
        uint offset = place % 8;
        bool res2;
        if (place / 8 < _nbytes)
            res2 = (const_raw()[place / 8] & (Bx80 >> offset)) != 0;
        else
            res2 = false;
        return res2;
    }

    /**
     * finds the overhead (unused number of bits) when allocating for size
     * bits.
     * @param size: number of bits
     * @return
     */
    static size_t overhead(uint size);

    /**
     * Deallocates memory stored on heap
     */
    void release()
    {
        if (_nbytes > PTR_SIZE)
            dealloc(_blob);
        _blob = nullptr;
        _nbytes = 0;
    }

    /**
     * Nice access to single bits
     * @param i: index to access
     * @return
     */
    uchar operator[](int i) const
    {
        if (i >= _nbytes) {
            return 0x0;
        }
        return const_raw()[i];
    }

    /**
     * Compares two wrappers. Assumes that smaller number of bytes also means
     * a smaller wrapper. Otherwise, compares byte by byte.
     * @param other: wrapper to compare to
     * @return -1 if other is smaller, 0 if same, 1 if other is larger
     */
    int cmp(const binarywrapper_t& other) const
    {
        if (_nbytes < other._nbytes)
            return -1;
        if (_nbytes > other._nbytes)
            return 1;

        size_t bcmp = std::min(_nbytes, other._nbytes);
        return std::memcmp(const_raw(), other.const_raw(), bcmp);
    }

    /**
     * If sizes differs, the comparison is done here.
     * If sizes match, compares byte by byte.
     * @param enc1
     * @param enc2
     * @return true if a match, false otherwise
     */
    friend bool operator==(const binarywrapper_t& enc1, const binarywrapper_t& enc2) { return enc1.cmp(enc2) == 0; }

    /**
     * If sizes differs, the comparison is done here.
     * If sizes match, compares byte by byte.
     * @param enc1
     * @param enc2
     * @return true if a match, false otherwise
     */
    friend bool operator<(const binarywrapper_t& enc1, const binarywrapper_t& enc2) { return enc1.cmp(enc2) < 0; }

    /**
     * If sizes differs, the comparison is done here.
     * If sizes match, compares byte by byte.
     * @param enc1
     * @param enc2
     * @return true if a match, false otherwise
     */
    friend bool operator!=(const binarywrapper_t& enc1, const binarywrapper_t& enc2) { return !(enc1 == enc2); }

    /**
     * If sizes differs, the comparison is done here.
     * If sizes match, compares byte by byte.
     * @param enc1
     * @param enc2
     * @return true if a match, false otherwise
     */
    friend bool operator>=(const binarywrapper_t& enc1, const binarywrapper_t& enc2) { return !(enc1 < enc2); }

    /**
     * If sizes differs, the comparison is done here.
     * If sizes match, compares byte by byte.
     * @param enc1
     * @param enc2
     * @return true if a match, false otherwise
     */
    friend bool operator>(const binarywrapper_t& enc1, const binarywrapper_t& enc2) { return enc2 < enc1; }

    /**
     * If sizes differs, the comparison is done here.
     * If sizes match, compares byte by byte.
     * @param enc1
     * @param enc2
     * @return true if a match, false otherwise
     */
    friend bool operator<=(const binarywrapper_t& enc1, const binarywrapper_t& enc2) { return enc2 >= enc1; }

private:
    static uchar* zallocate(size_t n)
    {
        if (n <= PTR_SIZE)
            return nullptr;
#ifndef NDEBUG
        size_t on = n;
#endif
        if (n % PTR_SIZE != 0) {
            n = (1 + (n / PTR_SIZE)) * PTR_SIZE;
            assert(n == on + (PTR_SIZE - (on % PTR_SIZE)));
        }
        assert(n % PTR_SIZE == 0);
        assert(on <= n);
        return (uchar*)calloc(n, 1);
    }

    static void dealloc(uchar* data) { free(data); }

    static uchar* offset(uchar* data, uint16_t size [[maybe_unused]])
    {
        //            if((size % __BW_BSIZE__) == 0) return data;
        //            else return &data[(__BW_BSIZE__ - (size % __BW_BSIZE__))];
        return data;
    }

    // blob of heap-allocated data
    uchar* _blob{nullptr};

    // number of bytes allocated on heap
    uint16_t _nbytes{0};

    // masks for single-bit access
};
}  // namespace ptrie
#endif /* BINARYWRAPPER_H */
