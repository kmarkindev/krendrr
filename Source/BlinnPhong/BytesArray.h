#pragma once

#include <span>

namespace krendrr::utils
{
    /**
     * Allows to get any array or container and represent it as span<byte>
     */
    class BytesArray : public std::span<const std::byte>
    {
    public:

        template<typename T>
        BytesArray(const std::span<const T>& AnySpan)
            : std::span<const std::byte> {reinterpret_cast<const std::byte*>(AnySpan.data()), AnySpan.size_bytes()}
        {

        }

        template<typename T, std::size_t Size>
        BytesArray(T (&AnyArray)[Size])
            : std::span<const std::byte> {reinterpret_cast<const std::byte*>(&AnyArray[0]), Size * sizeof(T)}
        {

        }

    };
}
