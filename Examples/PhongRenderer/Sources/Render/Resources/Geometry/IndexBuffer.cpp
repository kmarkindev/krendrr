#include "IndexBuffer.h"

void kRendrr::IndexBuffer::SetBufferSizeAndFormat(std::uint32_t BytesSize, DXGI_FORMAT Format, std::uint32_t IndicesCount)
{
    BufferSize = BytesSize;
    BufferFormat = Format;
    BufferIndicesCount = IndicesCount;
}

std::uint32_t kRendrr::IndexBuffer::GetIndicesCount() const
{
    return BufferIndicesCount;
}

D3D12_INDEX_BUFFER_VIEW kRendrr::IndexBuffer::GetIndexBufferView() const
{
    CheckInitialization();

    return {
        .BufferLocation = D3dBuffer->GetGPUVirtualAddress(),
        .SizeInBytes = BufferSize,
        .Format = BufferFormat
    };
}
