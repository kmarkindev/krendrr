#pragma once
#include <string_view>
#include <wrl/client.h>
#include <d3dx12/d3dx12.h>
#include <Runtime/RenderApi/Core/RenderApi.h>

namespace krendrr::Runtime::RenderApi::Core
{
    class PsoBuilderBase
    {
    protected:

        struct CompilationResult
        {
            bool bWasSuccessful {};
            Microsoft::WRL::ComPtr<ID3DBlob> ShaderBlob {};
        };

        CompilationResult CompileShader(
            const std::wstring_view& ShaderFilePath,
            const RenderApi& RenderApi,
            const std::string_view& EntryPoint,
            const std::string_view& ShaderModel
        ) const;
    };

    class GraphicsPsoBuilder final : public PsoBuilderBase
    {
    public:

        explicit GraphicsPsoBuilder(const RenderApi* NewRenderApi);

        /**
         * Creates builder instance
         */
        static GraphicsPsoBuilder Create(const RenderApi* NewRenderApi);

        // required, no default value
        GraphicsPsoBuilder& SetRootSignature(ID3D12RootSignature* NewRootSignature);
        // required, no default value
        GraphicsPsoBuilder& SetInputLayout(std::span<const D3D12_INPUT_ELEMENT_DESC> NewInputLayout);
        // required, no default value
        GraphicsPsoBuilder& SetVertexShader(const std::wstring_view& NewShaderFilePath,
            const std::string_view& NewEntryPoint = "VS_Main", const std::string_view& NewShaderModel = "vs_5_1");
        // required, no default value
        GraphicsPsoBuilder& SetPixelShader(const std::wstring_view& NewShaderFilePath,
            const std::string_view& NewEntryPoint = "PS_Main", const std::string_view& NewShaderModel = "ps_5_1");

        /**
         * Use this function to change count and formats for render targets.
         *
         * Note: max allowed number of formats is 8.
         *
         * Required, no default value
         */
        GraphicsPsoBuilder& SetRenderTargets(std::span<const DXGI_FORMAT> NewRenderTargetFormats, DXGI_FORMAT NewDepthStencilFormat = DXGI_FORMAT_UNKNOWN);
        GraphicsPsoBuilder& SetRenderTargets(std::initializer_list<DXGI_FORMAT> NewRenderTargetFormats, DXGI_FORMAT NewDepthStencilFormat = DXGI_FORMAT_UNKNOWN);

        /**
         * Use this function to change default blend state.
         * By default, it is CD3DX12_BLEND_DESC{D3D12_DEFAULT}.
         */
        GraphicsPsoBuilder& SetBlendState(D3D12_BLEND_DESC NewBlendState);

        /**
         * Use this function to change default rasterizer state.
         * By default, it is CD3DX12_RASTERIZER_DESC{D3D12_DEFAULT} with FrontCounterClockwise = true.
         */
        GraphicsPsoBuilder& SetRasterizerState(D3D12_RASTERIZER_DESC NewRasterizerState);

        /**
         * Use this function to change default depth-stencil state.
         * By default, it is CD3DX12_DEPTH_STENCIL_DESC{D3D12_DEFAULT}.
         */
        GraphicsPsoBuilder& SetDepthStencilState(D3D12_DEPTH_STENCIL_DESC NewDepthStencilState);

        /**
         * Use this function to change primitive topology type.
         * By default, it is D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE.
         */
        GraphicsPsoBuilder& SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE NewPrimitiveTopologyType);

        /**
         * Builds root signature using builder configuration
         */
        ID3D12PipelineState* Build(const std::wstring_view& PsoName = L"Graphics PSO");

    private:

        const RenderApi* RenderApi {};

        ID3D12RootSignature* RootSignature {};
        std::span<const D3D12_INPUT_ELEMENT_DESC> InputLayout {};

        std::wstring_view VertexShaderFilePath {};
        std::string_view VertexShaderEntryPoint {};
        std::string_view VertexShaderModel {};

        std::wstring_view PixelShaderFilePath {};
        std::string_view PixelShaderEntryPoint {};
        std::string_view PixelShaderModel {};

        std::span<const DXGI_FORMAT> RenderTargetFormats {};
        DXGI_FORMAT DepthStencilFormat {};

        D3D12_BLEND_DESC BlendState {CD3DX12_BLEND_DESC{D3D12_DEFAULT}};
        D3D12_RASTERIZER_DESC RasterizerState {CD3DX12_RASTERIZER_DESC{D3D12_DEFAULT}};
        D3D12_DEPTH_STENCIL_DESC DepthStencilState {CD3DX12_DEPTH_STENCIL_DESC{D3D12_DEFAULT}};

        D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType {D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE};
    };

    class ComputePsoBuilder final : public PsoBuilderBase
    {
    public:

        explicit ComputePsoBuilder(const RenderApi* NewRenderApi);

        /**
         * Creates builder instance
         */
        static ComputePsoBuilder Create(const RenderApi* NewRenderApi);

        // required, no default value
        ComputePsoBuilder& SetRootSignature(ID3D12RootSignature* NewRootSignature);

        // required, no default value
        ComputePsoBuilder& SetComputeShader(const std::wstring_view& NewShaderFilePath,
            const std::string_view& NewEntryPoint = "CS_Main", const std::string_view& NewShaderModel = "cs_5_1");

        /**
         * Builds root signature using builder configuration
         */
        ID3D12PipelineState* Build(const std::wstring_view& PsoName = L"Compute PSO");

    private:

        const RenderApi* RenderApi {};

        ID3D12RootSignature* RootSignature {};

        std::wstring_view ComputeShaderFilePath {};
        std::string_view ComputeShaderEntryPoint {};
        std::string_view ComputeShaderModel {};

    };
}

