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
#include <limits>
#include <stdexcept>

#include <cassert>
#include <cstdint>
#include <cstring>  // memcmp

namespace ptrie {
constexpr auto PTR_SIZE = sizeof(uintptr_t);  ///< the size of a pointer
constexpr auto BYTE_BITS = 8u;                ///< number of bits in a byte
using uint = unsigned int;
using uchar = unsigned char;

/// Strong type for denoting the number of bits
template <typename Count = uint>
struct Bits
{
    explicit constexpr Bits(Count bits): _bits{bits} {}
    constexpr Count bits() const { return _bits; }
    template <typename Bytes = uint16_t>
    constexpr Bytes bytes() const
    {
        const auto res = (_bits + BYTE_BITS - 1) / BYTE_BITS;  // roundup to ceiling as needed
        if (std::numeric_limits<Bytes>::max() < res)
            throw std::overflow_error{"Cannot fit " + std::to_string(_bits) + " bits"};
        return static_cast<Bytes>(res);
    }

private:
    Count _bits;
};
template <typename T>
Bits(T) -> Bits<T>;

inline Bits<> operator""_bits(unsigned long long bits)
{
    if (std::numeric_limits<uint>::max() < bits)
        throw std::overflow_error{"Cannot fit " + std::to_string(bits) + " bits"};
    return Bits{static_cast<uint>(bits)};
}

static_assert(Bits{0}.bytes() == 0);
static_assert(Bits{1}.bytes() == 1);
static_assert(Bits{7}.bytes() == 1);
static_assert(Bits{8}.bytes() == 1);
static_assert(Bits{9}.bytes() == 2);
static_assert(Bits{2040}.bytes<uchar>() == 255);
// static_assert(Bits{2041}.bytes<uchar>() == 256); // overflow
static_assert(Bits{2041}.bytes() == 256);

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)

/**
 * Wrapper for binary data. This provides easy access to individual bits,
 * heap allocation and comparison. Notice that one has to make sure to
 * explicitly call release() if one wishes to deallocate (possibly shared data).
 */
struct binarywrapper_t
{
    static constexpr uchar Bx80 = 0b10000000;  ///< bitmask with the highest bit set
    /// Default constructor does not allocate any data
    binarywrapper_t() = default;

    /// Allocates a room for at least size bits
    template <typename T>
    explicit binarywrapper_t(Bits<T> bits): _nbytes{bits.bytes()}, _blob{zallocate(_nbytes)}
    {}

    /// Allocates a room for at least size bits
    [[deprecated]] explicit binarywrapper_t(uint size): binarywrapper_t{Bits{size}} {}

    /**
     * Assign (not copy) raw data to pointer. Set number of bytes to size
     * @param raw some memory to point to
     * @param bits number of bits.
     */
    template <typename T>
    binarywrapper_t(uchar* raw, Bits<T> bits): _nbytes{bits.bytes()}, _blob{raw}
    {
        if (_nbytes <= PTR_SIZE)
            std::memcpy(const_raw(), raw, _nbytes);  // store in-place of the pointer
        //        assert(raw[0] == const_raw()[0]);
    }

    /**
     * Assign (not copy) raw data to pointer. Set number of bytes to size
     * @param raw some memory to point to
     * @param size number of bits.
     */
    [[deprecated]] binarywrapper_t(uchar* raw, uint size): binarywrapper_t{raw, Bits{size}} {}

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
            return offset(reinterpret_cast<uchar*>(&_blob), _nbytes);
        return offset(_blob, _nbytes);
    }

    /**
     * Raw access to data when in const setting
     * @return
     */
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    uchar* const_raw() const { return const_cast<binarywrapper_t*>(this)->raw(); }

    /**
     * Change value of place'th bit
     * @param place: index of bit to change
     * @param value: desired value
     */
    void set(const uint place, const bool value)
    {
        assert(place < _nbytes * BYTE_BITS);
        const uint byte_pos = place / BYTE_BITS;
        const uint bit_pos = place % BYTE_BITS;
        if (value) {
            raw()[byte_pos] |= (Bx80 >> bit_pos);
        } else {
            raw()[byte_pos] &= ~(Bx80 >> bit_pos);
        }
    }

    /**
     * Get value of the place'th bit
     * @param index: bit index
     * @return
     */
    bool at(const uint index) const
    {
        const uint byte_pos = index / BYTE_BITS;
        const uint bit_pos = index % BYTE_BITS;
        assert(byte_pos < _nbytes);
        const auto res = (const_raw()[byte_pos] & (Bx80 >> bit_pos)) != 0;
        return res;
    }

    /**
     * finds the overhead (unused number of bits) when allocating for size bits.
     * @param size: number of bits
     * @return
     */
    static size_t overhead(uint size)
    {
        size %= BYTE_BITS;
        if (size == 0)
            return 0;
        return BYTE_BITS - size;
    }

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
    uchar operator[](unsigned i) const
    {
        assert(i < _nbytes);
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

        const size_t bcmp = std::min(_nbytes, other._nbytes);
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
        const size_t on = n;
#endif
        if (n % PTR_SIZE != 0) {
            n = (1 + (n / PTR_SIZE)) * PTR_SIZE;
            assert(n == on + (PTR_SIZE - (on % PTR_SIZE)));
        }
        assert(n % PTR_SIZE == 0);
        assert(on <= n);
        return new uchar[n]{};  // NOLINT(cppcoreguidelines-owning-memory)
    }

    static void dealloc(const uchar* data) { delete[] data; }  // NOLINT(cppcoreguidelines-owning-memory)

    static uchar* offset(uchar* data, uint16_t size [[maybe_unused]])
    {
        //            if((size % PTR_SIZE) == 0) return data;
        //            else return &data[(PTR_SIZE - (size % PTR_SIZE))];
        return data;
    }

    uint16_t _nbytes{0};    ///< number of bytes allocated on heap
    uchar* _blob{nullptr};  ///< blob of heap-allocated data
};
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)

}  // namespace ptrie
#endif /* BINARYWRAPPER_H */
