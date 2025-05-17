#pragma once

#include <span>
#include "DataBufferBase.h"

namespace kRendrr
{
    class CommandList;

    class UploadBuffer : public DataBufferBase
    {
    public:

        UploadBuffer();

        template<typename T>
        void UploadData(const T& Container)
        {
            auto Span = std::span<const typename T::value_type>(Container);
            UploadData(Span);
        }

        template<typename T, size_t Size>
        void UploadData(std::span<T, Size> Span)
        {
            UploadDataInternal(Span.data(), Span.size_bytes());
        }

    private:

        void UploadDataInternal(const void* Data, size_t Size);

    };
}

