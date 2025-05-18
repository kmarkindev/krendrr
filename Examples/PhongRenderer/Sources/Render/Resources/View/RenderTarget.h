#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include "Sources/Render/Resources/RenderResource.h"

namespace kRendrr
{
    class RenderDevice;

    class RenderTarget : public RenderResource
    {
    public:

        RenderTarget() = default;
        
        void Initialize(const RenderDevice& RenderDevice);

        /**
         * Use this overload to initialize RTV using existing handles, e.g. when constructing RTV from Swap Chain
         */
        void Initialize(Microsoft::WRL::ComPtr<ID3D12Resource> RtvResource, D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle);

        Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const;

        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> Resource {};
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle {};

    };
}
