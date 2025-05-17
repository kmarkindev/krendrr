#pragma once

#include "Sources/Render/Resources/Buffers/DataBufferBase.h"

namespace kRendrr
{
    class RenderDevice;

    class IndexBuffer : public DataBufferBase
    {
    public:

        /**
         * This function has to be called with correct arguments
         * in order to use the buffer during rendering
         */
        void SetBufferSizeAndFormat(std::uint32_t BytesSize, DXGI_FORMAT Format, std::uint32_t IndicesCount);

        std::uint32_t GetIndicesCount() const;

        D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

    private:

        std::uint32_t BufferSize {};
        std::uint32_t BufferIndicesCount {};
        DXGI_FORMAT BufferFormat { DXGI_FORMAT_R32_UINT };

    };
}

