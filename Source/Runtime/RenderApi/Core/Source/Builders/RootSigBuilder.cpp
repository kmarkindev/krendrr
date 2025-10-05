#include "Runtime/RenderApi/Core/Builders/RootSigBuilder.h"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"

namespace krendrr::Runtime::RenderApi::Core
{
    RootSigBuilder::RootSigBuilder(const class RenderApi* NewRenderApi)
        : RenderApi(NewRenderApi)
    {
    }

    RootSigBuilder RootSigBuilder::Create(const class RenderApi* NewRenderApi)
    {
        return RootSigBuilder{NewRenderApi};
    }

    RootSigBuilder& RootSigBuilder::SetStaticSamplers(std::span<const D3D12_STATIC_SAMPLER_DESC> NewStaticSamplers)
    {
        StaticSamplers = std::move(NewStaticSamplers);

        return *this;
    }

    RootSigBuilder& RootSigBuilder::SetStaticSamplers(std::span<const CD3DX12_STATIC_SAMPLER_DESC> NewStaticSamplers)
    {
        StaticSamplers = std::move(NewStaticSamplers);

        return *this;
    }

    RootSigBuilder& RootSigBuilder::SetRootParams(std::span<const CD3DX12_ROOT_PARAMETER> NewRootParams)
    {
        RootParams = std::move(NewRootParams);

        return *this;
    }

    RootSigBuilder& RootSigBuilder::SetRootParams(std::span<const D3D12_ROOT_PARAMETER> NewRootParams)
    {
        RootParams = std::move(NewRootParams);

        return *this;
    }

    RootSigBuilder& RootSigBuilder::SetFlags(D3D12_ROOT_SIGNATURE_FLAGS NewFlags)
    {
        Flags = std::move(NewFlags);

        return *this;
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSigBuilder::Build(const std::wstring_view& RootSigName)
    {
        if (!RenderApi)
        {
            // TODO: log error
            return {};
        }

        CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc {};

        std::visit(
            [&](auto& RootParams, auto& StaticSamplers)
            {
                RootSignatureDesc.Init(
                    RootParams.size(),
                    RootParams.data(),
                    StaticSamplers.size(),
                    StaticSamplers.data(),
                    Flags
                );
            },
            RootParams,
            StaticSamplers
        );

        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureBlob {};
        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrorBlob {};

        const HRESULT RootSigSerResult = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &RootSignatureBlob, &RootSignatureErrorBlob);
        if (FAILED(RootSigSerResult))
        {
            std::string error (static_cast<const char*>(RootSignatureErrorBlob->GetBufferPointer()), RootSignatureErrorBlob->GetBufferSize());

            // TODO: log error

            __debugbreak();

            return {};
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSig {};

        CHECKED(
            RenderApi->GetDevice()
                ->CreateRootSignature(
                    0,
                    RootSignatureBlob->GetBufferPointer(),
                    RootSignatureBlob->GetBufferSize(),
                    IID_PPV_ARGS(&RootSig)
                ),
            "Failed to create root signature"
        )

        CHECKED(
            RootSig->SetName(RootSigName.data()),
            "Failed to set root signature name"
        )

        return RootSig;
    }
}
