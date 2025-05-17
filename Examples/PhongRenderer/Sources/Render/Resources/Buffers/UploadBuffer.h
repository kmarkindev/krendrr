#pragma once

#include <span>
#include "DataBufferBase.h"

namespace kRendrr
{
    class CommandQueue;
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

        /**
         * Creates a Command Allocator, Command List and Fence, uploads data and waits for completion.
         * Highly unoptimized, but allows to make prototypes faster.
         *
         * SizeBytes specifies how much data needs to be copied from upload buffer to target buffer,
         * in case you reuse upload buffer for multiple uploads.
         */
        void UploadDataToBuffer(const RenderDevice& RenderDevice, CommandQueue& CommandQueue, DataBufferBase& TargetBuffer, size_t SizeBytes);

    private:

        void UploadDataInternal(const void* Data, size_t Size);

    };
}

