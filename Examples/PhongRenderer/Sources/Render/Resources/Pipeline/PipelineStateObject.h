#pragma once

#include <d3d12.h>
#include <memory>
#include <wrl/client.h>

#include "RootSignature.h"
#include "Sources/Render/Resources/RenderResource.h"

namespace kRendrr
{
    class Shader;
}

namespace kRendrr
{
    class RenderDevice;

    class PipelineStateObject : public RenderResource
    {
    public:

        struct PsoInitParams
        {
            const Shader& VertexShader;
            const Shader& PixelShader;
            D3D12_INPUT_LAYOUT_DESC InputLayout;
            D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType { D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
        };

        explicit PipelineStateObject(std::shared_ptr<RootSignature> RootSignature);

        void Initialize(const RenderDevice& RenderDevice, const PsoInitParams& Params);

        Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPso() const;

    private:

        std::shared_ptr<RootSignature> RootSignature {};
        Microsoft::WRL::ComPtr<ID3D12PipelineState> D3dPso {};

    };
}

