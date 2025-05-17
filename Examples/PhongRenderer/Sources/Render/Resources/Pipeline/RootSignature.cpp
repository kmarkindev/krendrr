#include "RootSignature.h"
#include <d3dx12/d3dx12_root_signature.h>
#include "Sources/Render/RenderDevice.h"
#include "Sources/Utils/HResultCheck.h"

namespace kRendrr
{
    void RootSignature::Initialize(const RenderDevice& RenderDevice, std::span<CD3DX12_ROOT_PARAMETER> RootParams, std::span<D3D12_STATIC_SAMPLER_DESC> Samplers,
        D3D12_ROOT_SIGNATURE_FLAGS Flags)
    {
        CheckInitialization(false);

        CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc {};
        RootSignatureDesc.Init(RootParams.size(), RootParams.data(), Samplers.size(), Samplers.data(), Flags);

        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureBlob {};
        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrorBlob {};

        D3D12SerializeRootSignature(
            &RootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1_0,
            &RootSignatureBlob,
            &RootSignatureErrorBlob
        ) >> HResultCheck {};

        RenderDevice.GetDevice()
            ->CreateRootSignature(
                0,
                RootSignatureBlob->GetBufferPointer(),
                RootSignatureBlob->GetBufferSize(),
                IID_PPV_ARGS(&D3dRootSignature)
            ) >> HResultCheck {};

        MarkAsInitialized();
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature::GetRootSignature() const
    {
        CheckInitialization();
        return D3dRootSignature;
    }
}
