#include "PipelineStateObject.h"

#include <d3dx12/d3dx12_core.h>

#include "Sources/Render/RenderDevice.h"
#include "Sources/Render/Resources/Shaders/Shader.h"
#include "Sources/Utils/HResultCheck.h"

kRendrr::PipelineStateObject::PipelineStateObject(std::shared_ptr<kRendrr::RootSignature> RootSignature)
    : RootSignature(std::move(RootSignature))
{

}

void kRendrr::PipelineStateObject::Initialize(const RenderDevice& RenderDevice, const PsoInitParams& Params)
{
    CheckInitialization(false);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc {
        .pRootSignature = RootSignature->GetRootSignature().Get(),
        .VS = Params.VertexShader.GetShaderByteCode(),
        .PS = Params.PixelShader.GetShaderByteCode(),
        .BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
        .SampleMask = UINT_MAX,
        .RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
        .DepthStencilState = {},
        .InputLayout = Params.InputLayout,
        .IBStripCutValue = {},
        .PrimitiveTopologyType = Params.TopologyType,
        .NumRenderTargets = 1,
        .RTVFormats = {
            DXGI_FORMAT_R8G8B8A8_UNORM
        },
        .DSVFormat = {},
        .SampleDesc = {
            .Count = 1,
            .Quality = 0
        },
        .CachedPSO = {}
    };

    RenderDevice.GetDevice()
        ->CreateGraphicsPipelineState(
            &PsoDesc,
            IID_PPV_ARGS(&D3dPso)
        ) >> HResultCheck {};

    MarkAsInitialized();
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> kRendrr::PipelineStateObject::GetPso() const
{
    CheckInitialization();

    return D3dPso;
}
