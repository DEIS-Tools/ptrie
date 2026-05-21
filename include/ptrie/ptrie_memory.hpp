#ifndef PTRIE_PTRIE_MEMORY_HPP
#define PTRIE_PTRIE_MEMORY_HPP

#include <algorithm>    // copy (instead of memcpy)
#include <bit>          // bit_cast
#include <type_traits>  // is_void

#include <cstdint>  // uint32_t
#include <cassert>

/**
 * Direct memory manipulation utilities
 */
namespace ptrie {
using uint = uint32_t;
using uchar = unsigned char;

template <typename As = uchar, typename T>
constexpr auto* as_array(T* data)
{
    static_assert(std::is_void_v<T> || std::has_unique_object_representations_v<T>);
    using Res = std::conditional_t<std::is_const_v<T>, const As, As>;
    return std::bit_cast<Res*>(data);  // NOLINT(bugprone-bitwise-pointer-cast)
}

/// Solves alignment when reading from compressed storage, see std::bit_cast
template <typename T>
constexpr T mem_load(const void* memory)
{
    T res;
    std::copy_n(as_array(memory), sizeof(T), as_array(&res));
    return res;
}

/// Solves alignment warnings when writing into compressed storage
template <typename T>
constexpr void mem_store(void* memory, const T& value)
{
    std::copy_n(as_array(&value), sizeof(T), as_array(memory));
}

/// Solves alignment warnings when writing into compressed storage
template <typename T>
constexpr void mem_copy(const T* src, T* dest, size_t count)
{
    std::copy_n(as_array(src), sizeof(T) * count, as_array(dest));
}

/// Iterates through bytes of the KEY. If KEY contains padding, then a custom specialization should be used.
/// TODO: replace with byte_view pseudo container with its own iterators and subscript accessors.
template <typename KEY>
struct byte_iterator
{
    static constexpr uchar& access(KEY* data, size_t id)
    {
        static_assert(std::has_unique_object_representations_v<KEY>,
                      "use byte_iterator specialization for KEY instead");
        assert(data);
        return as_array(data)[id];
    }

    static constexpr const uchar& const_access(const KEY* data, size_t id)
    {
        static_assert(std::has_unique_object_representations_v<KEY>,
                      "use byte_iterator specialization for KEY instead");
        assert(data);
        return as_array(data)[id];
    }

    static constexpr size_t element_size()
    {
        static_assert(std::has_unique_object_representations_v<KEY>,
                      "use byte_iterator specialization for KEY instead");
        return sizeof(KEY);
    }

    static constexpr bool continuous() { return std::has_unique_object_representations_v<KEY>; }
    [[deprecated]] static constexpr bool continious() { return continuous(); }
    // add read_blob, write_blob
};
}  // namespace ptrie

#endif  // PTRIE_PTRIE_MEMORY_HPP
