#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include "Sources/Render/Resources/RenderResource.h"
#include <cinttypes>
#include <d3dx12/d3dx12_root_signature.h>

namespace kRendrr
{
    class RenderDevice;

    class DescriptorHeap : public RenderResource
    {
    public:

        void Initialize(const RenderDevice& RenderDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type, std::uint32_t DescriptorsCount, bool bShaderVisible);

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const;

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(std::int32_t Index) const;

        CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(std::int32_t Index) const;

    private:

        size_t DescriptorSize {};

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> D3dDescriptorHeap;

        bool bShaderVisible {false};

    };
}

