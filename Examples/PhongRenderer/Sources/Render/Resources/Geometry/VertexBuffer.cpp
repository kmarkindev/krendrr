#include "VertexBuffer.h"

void kRendrr::VertexBuffer::SetBufferSideAndStride(std::uint32_t Size, std::uint32_t Stride, std::uint32_t VertexCount)
{
    BufferSize = Size;
    BufferStride = Stride;
    BufferVertexCount = VertexCount;
}

D3D12_VERTEX_BUFFER_VIEW kRendrr::VertexBuffer::GetVertexBufferView() const
{
    CheckInitialization();

    return {
        .BufferLocation = D3dBuffer->GetGPUVirtualAddress(),
        .SizeInBytes = BufferSize,
        .StrideInBytes = BufferStride
    };
}
