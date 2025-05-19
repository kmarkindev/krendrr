#pragma once

#include <d3d12.h>
#include <span>
#include <d3dx12/d3dx12_root_signature.h>
#include <wrl/client.h>
#include "Sources/Render/Resources/RenderResource.h"

struct CD3DX12_ROOT_PARAMETER;

namespace kRendrr
{
    class RenderDevice;

    class RootSignature : public RenderResource
    {
    public:

        void Initialize(
            const RenderDevice& RenderDevice,
            std::span<CD3DX12_ROOT_PARAMETER> RootParams,
            std::span<CD3DX12_STATIC_SAMPLER_DESC> Samplers,
            D3D12_ROOT_SIGNATURE_FLAGS Flags
        );

        Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12RootSignature> D3dRootSignature {};

    };
}

