#pragma once

#include "Sources/Render/Resources/Buffers/DataBufferBase.h"

namespace kRendrr
{
    class RenderDevice;

    class VertexBuffer : public DataBufferBase
    {
    public:

        /**
         * This function has to be called with correct arguments
         * in order to use the buffer during rendering
         */
        void SetBufferSideAndStride(std::uint32_t Size, std::uint32_t Stride, std::uint32_t VertexCount);

        D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

    private:

        std::uint32_t BufferSize {};
        std::uint32_t BufferStride {};
        std::uint32_t BufferVertexCount {};

    };
}

