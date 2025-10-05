#pragma once

#include <variant>
#include <Runtime/RenderApi/Core/RenderApi.h>
#include <d3dx12/d3dx12.h>

namespace krendrr::Runtime::RenderApi::Core
{

    class RootSigBuilder
    {
    public:

        explicit RootSigBuilder(const RenderApi* NewRenderApi);

        /**
         * Creates builder instance
         */
        static RootSigBuilder Create(const RenderApi* NewRenderApi);

        RootSigBuilder& SetStaticSamplers(std::span<const D3D12_STATIC_SAMPLER_DESC> NewStaticSamplers);
        RootSigBuilder& SetStaticSamplers(std::span<const CD3DX12_STATIC_SAMPLER_DESC> NewStaticSamplers);

        RootSigBuilder& SetRootParams(std::span<const D3D12_ROOT_PARAMETER> NewRootParams);
        RootSigBuilder& SetRootParams(std::span<const CD3DX12_ROOT_PARAMETER> NewRootParams);

        RootSigBuilder& SetFlags(D3D12_ROOT_SIGNATURE_FLAGS NewFlags);

        ID3D12RootSignature* Build(const std::wstring_view& RootSigName = L"Root Signature");

    private:

        const RenderApi* RenderApi {};

        std::variant<std::span<const D3D12_ROOT_PARAMETER>, std::span<const CD3DX12_ROOT_PARAMETER>> RootParams {};
        std::variant<std::span<const D3D12_STATIC_SAMPLER_DESC>, std::span<const CD3DX12_STATIC_SAMPLER_DESC>> StaticSamplers {};

        D3D12_ROOT_SIGNATURE_FLAGS Flags {};

    };

}

