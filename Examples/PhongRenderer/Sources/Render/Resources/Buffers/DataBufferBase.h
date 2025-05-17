#pragma once

#include "Sources/Render/Resources/RenderResource.h"
#include <cinttypes>
#include <d3d12.h>
#include <wrl/client.h>

namespace kRendrr
{
    class RenderDevice;

    class DataBufferBase : public RenderResource
    {
    public:

        void Initialize(const RenderDevice& RenderDevice, std::uint64_t BufferSize);

        Microsoft::WRL::ComPtr<ID3D12Resource> GetBuffer() const;

        [[nodiscard]] std::uint64_t GetBufferSize() const;

    protected:

        D3D12_HEAP_TYPE BufferHeapType { D3D12_HEAP_TYPE_DEFAULT };
        Microsoft::WRL::ComPtr<ID3D12Resource> D3dBuffer {};

    };
}

