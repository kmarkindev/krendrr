#include "Runtime/RenderApi/Core/Builders/PsoBuilder.h"
#include <d3dcompiler.h>
#include <filesystem>

#include "Runtime/RenderApi/Core/ApiCallCheck.h"
#include "Runtime/RenderApi/Core/ContentFolderD3DInclude.h"

namespace krendrr::Runtime::RenderApi::Core
{
    PsoBuilderBase::CompilationResult PsoBuilderBase::CompileShader(
        const std::wstring_view& ShaderFilePath,
        const RenderApi& RenderApi,
        const std::string_view& EntryPoint,
        const std::string_view& ShaderModel
    ) const
    {
        Microsoft::WRL::ComPtr<ID3DBlob> CompilationErrorBlob {};
        Microsoft::WRL::ComPtr<ID3DBlob> ShaderBlob {};

        ContentFolderD3dInclude ShaderIncludeInterface {};

        if (!std::filesystem::exists(ShaderFilePath))
        {
            // TODO: log error

            return {};
        }

        const HRESULT CompileResult = D3DCompileFromFile(
            ShaderFilePath.data(),
            nullptr,
            &ShaderIncludeInterface,
            EntryPoint.data(),
            ShaderModel.data(),
            RenderApi.GetShaderCompileFlags(),
            0,
            &ShaderBlob,
            &CompilationErrorBlob
        );

        if(FAILED(CompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error(static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());

            // TODO: log error

            __debugbreak();

            return {};
        }

        return {
            .bWasSuccessful = true,
            .ShaderBlob = std::move(ShaderBlob)
        };
    }

    GraphicsPsoBuilder::GraphicsPsoBuilder(const class RenderApi* NewRenderApi)
        : RenderApi(NewRenderApi)
    {
        RasterizerState.FrontCounterClockwise = true;
    }

    GraphicsPsoBuilder GraphicsPsoBuilder::Create(const class RenderApi* NewRenderApi)
    {
        return GraphicsPsoBuilder {NewRenderApi};
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetRootSignature(ID3D12RootSignature* NewRootSignature)
    {
        RootSignature = std::move(NewRootSignature);

        return *this;
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetInputLayout(std::span<const D3D12_INPUT_ELEMENT_DESC> NewInputLayout)
    {
        InputLayout = std::move(NewInputLayout);

        return *this;
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetVertexShader(const std::wstring_view& NewShaderFilePath, const std::string_view& NewEntryPoint, const std::string_view& NewShaderModel)
    {
        VertexShaderFilePath = std::move(NewShaderFilePath);
        VertexShaderEntryPoint = std::move(NewEntryPoint);
        VertexShaderModel = std::move(NewShaderModel);

        return *this;
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetPixelShader(const std::wstring_view& NewShaderFilePath, const std::string_view& NewEntryPoint, const std::string_view& NewShaderModel)
    {
        PixelShaderFilePath = std::move(NewShaderFilePath);
        PixelShaderEntryPoint = std::move(NewEntryPoint);
        PixelShaderModel = std::move(NewShaderModel);

        return *this;
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetBlendState(D3D12_BLEND_DESC NewBlendState)
    {
        BlendState = std::move(NewBlendState);

        return *this;
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetRasterizerState(D3D12_RASTERIZER_DESC NewRasterizerState)
    {
        RasterizerState = std::move(NewRasterizerState);

        return *this;
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetDepthStencilState(D3D12_DEPTH_STENCIL_DESC NewDepthStencilState)
    {
        DepthStencilState = std::move(NewDepthStencilState);

        return *this;
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE NewPrimitiveTopologyType)
    {
        PrimitiveTopologyType = std::move(NewPrimitiveTopologyType);

        return *this;
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetRenderTargets(std::span<const DXGI_FORMAT> NewRenderTargetFormats, DXGI_FORMAT NewDepthStencilFormat)
    {
        RenderTargetFormats = std::move(NewRenderTargetFormats);
        DepthStencilFormat = std::move(NewDepthStencilFormat);

        return *this;
    }

    GraphicsPsoBuilder& GraphicsPsoBuilder::SetRenderTargets(std::initializer_list<DXGI_FORMAT> NewRenderTargetFormats, DXGI_FORMAT NewDepthStencilFormat)
    {
        const std::span Formats = NewRenderTargetFormats;
        return SetRenderTargets(Formats, NewDepthStencilFormat);
    }

    Microsoft::WRL::ComPtr<ID3D12PipelineState> GraphicsPsoBuilder::Build(const std::wstring_view& PsoName)
    {
        if (!RenderApi)
        {
            // TODO: log error
            return {};
        }

        if (!RootSignature || InputLayout.empty() || VertexShaderFilePath.empty() || PixelShaderFilePath.empty())
        {
            // TODO: log error "not all required values are set"
            return {};
        }

        if (DepthStencilFormat == DXGI_FORMAT_UNKNOWN && RenderTargetFormats.empty())
        {
            // TODO: log error "you must set at least one render target or depth format. empty depth format and render targets are not allowed"
            return {};
        }

        const CompilationResult VertexShaderCompile = CompileShader(
            VertexShaderFilePath,
            *RenderApi,
            VertexShaderEntryPoint,
            VertexShaderModel
        );

        if (!VertexShaderCompile.bWasSuccessful)
        {
            // TODO: log error
            return {};
        }

        const CompilationResult PixelShaderCompile = CompileShader(
            PixelShaderFilePath,
            *RenderApi,
            PixelShaderEntryPoint,
            PixelShaderModel
        );

        if (!PixelShaderCompile.bWasSuccessful)
        {
            // TODO: log error
            return {};
        }

        std::array<DXGI_FORMAT, 8> Formats {};
        for (int i = 0; i < 8; i++)
        {
            const DXGI_FORMAT Format = i < RenderTargetFormats.size()
                ? RenderTargetFormats[i]
                : DXGI_FORMAT_UNKNOWN;

            Formats[i] = Format;
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc {
            .pRootSignature = RootSignature,
            .VS = CD3DX12_SHADER_BYTECODE(VertexShaderCompile.ShaderBlob.Get()),
            .PS = CD3DX12_SHADER_BYTECODE(PixelShaderCompile.ShaderBlob.Get()),
            .BlendState = BlendState,
            .SampleMask = UINT_MAX,
            .RasterizerState = RasterizerState,
            .DepthStencilState = DepthStencilFormat != DXGI_FORMAT_UNKNOWN ? DepthStencilState : D3D12_DEPTH_STENCIL_DESC{},
            .InputLayout = {
                .pInputElementDescs = InputLayout.data(),
                .NumElements = static_cast<UINT>(InputLayout.size()),
            },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = static_cast<UINT>(RenderTargetFormats.size()),
            .DSVFormat = DepthStencilFormat,
            .SampleDesc = {
                .Count = 1
            }
        };

        std::memcpy(PsoDesc.RTVFormats, Formats.data(), sizeof(Formats));

        Microsoft::WRL::ComPtr<ID3D12PipelineState> Pso {};

        CHECKED(
            RenderApi->GetDevice()
                ->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&Pso)),
            "Failed to create graphics PSO"
        )

        CHECKED(
            Pso->SetName(PsoName.data()),
            "Failed to set PSO name"
        )

        return Pso;
    }

    ComputePsoBuilder ComputePsoBuilder::Create(const class RenderApi* NewRenderApi)
    {
        return ComputePsoBuilder {NewRenderApi};
    }

    ComputePsoBuilder& ComputePsoBuilder::SetRootSignature(ID3D12RootSignature* NewRootSignature)
    {
        RootSignature = std::move(NewRootSignature);

        return *this;
    }

    ComputePsoBuilder::ComputePsoBuilder(const class RenderApi* NewRenderApi)
        : RenderApi(NewRenderApi)
    {

    }

    ComputePsoBuilder& ComputePsoBuilder::SetComputeShader(const std::wstring_view& NewShaderFilePath, const std::string_view& NewEntryPoint,
        const std::string_view& NewShaderModel)
    {
        ComputeShaderFilePath = std::move(NewShaderFilePath);
        ComputeShaderEntryPoint = std::move(NewEntryPoint);
        ComputeShaderModel = std::move(NewShaderModel);

        return *this;
    }

    Microsoft::WRL::ComPtr<ID3D12PipelineState> ComputePsoBuilder::Build(const std::wstring_view& PsoName)
    {
        if (!RenderApi)
        {
            // TODO: log error
            return {};
        }

        if (!RootSignature || ComputeShaderFilePath.empty())
        {
            // TODO: log error "not all required values were specified"
            return {};
        }

        const CompilationResult ComputeShaderCompile = CompileShader(
            ComputeShaderFilePath,
            *RenderApi,
            ComputeShaderEntryPoint,
            ComputeShaderModel
        );

        if (!ComputeShaderCompile.bWasSuccessful)
        {
            // TODO: log error
            return {};
        }

        Microsoft::WRL::ComPtr<ID3D12PipelineState> Pso {};

        D3D12_COMPUTE_PIPELINE_STATE_DESC PsoDesc {
            .pRootSignature = RootSignature,
            .CS = CD3DX12_SHADER_BYTECODE {ComputeShaderCompile.ShaderBlob.Get()},
        };

        const HRESULT PsoCreationResult = RenderApi->GetDevice()
            ->CreateComputePipelineState(&PsoDesc, IID_PPV_ARGS(&Pso));

        if (FAILED(PsoCreationResult))
        {
            // TODO: log error
            return {};
        }

        if (FAILED(Pso->SetName(PsoName.data())))
        {
            // TODO: log error
            return {};
        }

        return Pso;
    }
}
