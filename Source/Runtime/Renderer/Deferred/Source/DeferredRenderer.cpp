#include "Runtime/Renderer/Deferred/DeferredRenderer.h"
#include <array>
#include <d3dcompiler.h>

#include "Runtime/ModelLoader/Loader.h"
#include "Runtime/Renderer/Core/Scene/Scene.h"
#include "Runtime/Renderer/Core/Scene/SceneView.h"
#include "Runtime/Renderer/Core/TexturedMesh/Texture.h"
#include "nvtx3/nvtx3.hpp"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"
#include "Runtime/RenderApi/Core/ConstBufferHelper.h"
#include "Runtime/RenderApi/Core/ContentFolderD3DInclude.h"
#include "Runtime/Renderer/Core/Lights/PointLight.h"

namespace krendrr::Runtime::Renderer::Deferred
{

bool DeferredRenderer::Initialize(std::shared_ptr<RenderApi::Core::RenderApi> NewRenderApi, std::shared_ptr<Core::Scene> NewScene)
{
    nvtx3::scoped_range InitRange {"Deferred Renderer: Initialize"};

    RenderApi = std::move(NewRenderApi);
    Scene = std::move(NewScene);

    RenderThreadPool.Initialize();

    CHECKED(
        RenderApi->GetDevice()
            ->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&FrameFence)),
        "Failed to create Fence"
    )

    if (!InitPrePostRender())
        return false;

    if (!InitBasicMeshes())
        return false;

    if (!InitEmptyTexture())
        return false;

    if (!InitializeGeometryPass())
        return false;

    if (!InitLightPass())
        return false;

    if (!InitAmbientDirectionalLightPass())
        return false;

    if (!InitPostProcessingPass())
        return false;

    if (!InitPointLightShadowCubeMapPass())
        return false;

    if (!InitPointLightVolumePass())
        return false;

    if (!WaitDirectQueue())
        return false;

    return true;
}

bool DeferredRenderer::Render(const std::span<Core::SceneView>& SceneViews)
{
    if (Scene->GetPointLights().size() > PointLightVolumePassData.MAX_DYNAMIC_POINT_LIGHTS_COUNT)
    {
        // TODO: add error log
        return false;
    }

    for (const Core::SceneView& SceneView : SceneViews)
    {
        if (!UpdateFrameDataConstantBuffer(SceneView))
            return false;

        if (!UpdateTexturedMeshConstantBuffers())
            return false;

        if (!UpdatePointLightConstantBuffers())
            return false;

        if (!PreRender(SceneView))
            return false;

        if (!InitGBufferForView(SceneView))
            return false;

        if (!GeometryPass(SceneView))
            return false;

        if (!TransitionGBufferFromRenderTargetToReadState())
            return false;

        if (!PrepareLightPassData(SceneView))
            return false;

        if (!AmbientDirectionalLightPass(SceneView))
            return false;

        if (!PointLightShadowCubeMapsPass())
            return false;

        if (!PointLightVolumesPass(SceneView))
            return false;

        if (!TransitionLightPassFromRenderTargetToReadState())
            return false;

        if (!PostProcessingPass(SceneView))
            return false;

        if (!TransitionLightPassFromReadToRenderTargetState())
            return false;

        if (!PostRender(SceneView))
            return false;

        if (!TransitionGBufferFromReadToRenderTargetState())
            return false;

        if (!WaitDirectQueue())
            return false;
    }

    return true;
}

bool DeferredRenderer::UpdateFrameDataConstantBuffer(const Core::SceneView& SceneView)
{
    nvtx3::scoped_range ConstBufUpdateRange {"Update Frame Data Constant Buffer"};

    // Create buffer if not created
    if (FrameData.ConstantBuffer == nullptr)
    {
        if (!InitializeConstantBuffer<ConstBuff_Frame>(*RenderApi, FrameData.ConstantBuffer, FrameData.CpuSrvHeap, L"Frame Data Constant Buffer"))
            return false;
    }

    // Update buffer

    ConstBuff_Frame* Buffer {};
    CHECKED_S(FrameData.ConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&Buffer)))

    *Buffer = {
        .ViewMatrix = SceneView.GetViewMatrix(),
        .ProjectionMatrix = SceneView.GetProjectionMatrix(),
        .bHasAmbientLight = Scene->HasAmbientLight(),
        .AmbientColor = Scene->GetAmbientLightData().Color,
        .AmbientIntensity = Scene->GetAmbientLightData().Intensity,
        .DirectionalColor = Scene->GetDirectionalLightData().Color,
        .bHasDirectionalLight = Scene->HasDirectionalLight(),
        .DirectionalDir = Scene->GetDirectionalLightData().Direction,
        .DirectionalIntensity = Scene->GetDirectionalLightData().Intensity,
        .CameraPosition = SceneView.GetPosition(),
        .ViewportSize = SceneView.GetViewportSize(),
    };

    FrameData.ConstantBuffer->Unmap(0, nullptr);

    return true;
}

bool DeferredRenderer::UpdateTexturedMeshConstantBuffers()
{
    nvtx3::scoped_range ConstBufUpdateRange {"Update Textured Mesh Constant Buffers"};

    for (auto& TexturedMesh : Scene->GetTexturedMeshes())
    {
        if (!TexturedMesh->UpdateConstantBuffer(*RenderApi))
            return false;
    }

    return true;
}

bool DeferredRenderer::UpdatePointLightConstantBuffers()
{
    nvtx3::scoped_range ConstBufUpdateRange {"Update Point Light Constant Buffers"};

    for (auto& PointLight : Scene->GetPointLights())
    {
        if (!PointLight->UpdateConstantBuffer(*RenderApi.get()))
            return false;
    }

    return true;
}

bool DeferredRenderer::InitializeGeometryPass()
{
    // Create Root Signature
    {
        CD3DX12_ROOT_PARAMETER RootParams[3] {};

        // Descriptor table with textured mesh textures
        CD3DX12_DESCRIPTOR_RANGE TexturedMeshTexturesRange {};
        TexturedMeshTexturesRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT, 0);

        RootParams[0].InitAsDescriptorTable(1, &TexturedMeshTexturesRange);

        // Frame constant buffer
        RootParams[1].InitAsConstantBufferView(0);

        // Textured mesh constant buffer
        RootParams[2].InitAsConstantBufferView(1);

        const auto& StaticSamplers = GetCommonStaticSamplers();

        CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc {};
        RootSignatureDesc.Init(
            std::size(RootParams),
            RootParams,
            StaticSamplers.size(),
            StaticSamplers.data(),
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureBlob {};
        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrorBlob {};

        HRESULT RootSigSerResult = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &RootSignatureBlob, &RootSignatureErrorBlob);
        if (FAILED(RootSigSerResult))
        {
            std::string error (static_cast<const char*>(RootSignatureErrorBlob->GetBufferPointer()), RootSignatureErrorBlob->GetBufferSize());
            __debugbreak();
            return false;
        }

        CHECKED(
            RenderApi->GetDevice()
                ->CreateRootSignature(
                    0,
                    RootSignatureBlob->GetBufferPointer(),
                    RootSignatureBlob->GetBufferSize(),
                    IID_PPV_ARGS(&GeometryPassData.RootSignature)
                ),
            "Can't create root signature"
        )
    }

    // Load Shaders
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader {};
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader {};
    {
        Microsoft::WRL::ComPtr<ID3DBlob> CompilationErrorBlob {};

        RenderApi::Core::ContentFolderD3dInclude VertexShaderInclude {};

        HRESULT VSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/GeometryPass.hlsl",
            nullptr, &VertexShaderInclude, "VS_Main", "vs_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &VertexShader, &CompilationErrorBlob);

        if(FAILED(VSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }

        RenderApi::Core::ContentFolderD3dInclude PixelShaderInclude {};

        HRESULT PSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/GeometryPass.hlsl",
            nullptr, &PixelShaderInclude, "PS_Main", "ps_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &PixelShader, &CompilationErrorBlob);

        if(FAILED(PSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }
    }

    // Create PSO
    {
        auto RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        RasterizerState.FrontCounterClockwise = true;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc {
            .pRootSignature = GeometryPassData.RootSignature.Get(),
            .VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get()),
            .PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get()),
            .BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            .SampleMask = UINT_MAX,
            .RasterizerState = RasterizerState,
            .DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
            .InputLayout = {
                .pInputElementDescs = RenderApi->GetCommonMeshBufferLayout().Layout.data(),
                .NumElements = static_cast<UINT>(RenderApi->GetCommonMeshBufferLayout().Layout.size())
            },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = GBuffer.TEXTURES_COUNT,
            .RTVFormats = {
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_R8G8B8A8_UNORM,
            },
            .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {
                .Count = 1
            }
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&GeometryPassData.PipelineState)),
            "Failed to create PSO"
        )

        GeometryPassData.PipelineState->SetName(L"Geometry Pass PSO");
    }

    // Create GPU srv heap for textured mesh texture handles
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = GeometryPassData.PARALLEL_DRAWS_COUNT_ALLOWED * GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&GeometryPassData.GpuDescriptorHeap)),
            "Can't create descriptor heap"
        )
    }

    // Create command list
    {
        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&GeometryPassData.DrawCommandAllocator)),
            "Can't create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    GeometryPassData.DrawCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&GeometryPassData.DrawCommandList)),
            "Can't create command list"
        )

        CHECKED_S(GeometryPassData.DrawCommandList->Close());
    }

    return true;
}

bool DeferredRenderer::GeometryPass(const Core::SceneView& SceneView)
{
    nvtx3::scoped_range PassRange {"Geometry Pass"};

    const unsigned RtvHandleIncrement = RenderApi->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const unsigned SrvHandleIncrement = RenderApi->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, GBuffer.TEXTURES_COUNT> RtvHandles {};
    for (int i = 0; i < GBuffer.TEXTURES_COUNT; ++i)
    {
        RtvHandles[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE{GBuffer.CpuRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), i, RtvHandleIncrement};
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = GBuffer.CpuDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    auto& CommandAllocator = GeometryPassData.DrawCommandAllocator;
    auto& CommandList = GeometryPassData.DrawCommandList;

    auto ExecuteCommandList = [this, CommandList]() -> bool
    {
        CHECKED_S(CommandList->Close())

        ID3D12CommandList* CommandLists[] = {CommandList.Get()};

        RenderApi->GetDirectQueue()
            ->ExecuteCommandLists(1, CommandLists);

        return true;
    };

    auto ResetCommandList = [this, &CommandAllocator, &CommandList, &SceneView, &RtvHandles, &DsvHandle]() -> bool
    {
        CHECKED_S(CommandAllocator->Reset());
        CHECKED_S(CommandList->Reset(CommandAllocator.Get(), GeometryPassData.PipelineState.Get()));

        // Setup pipeline

        CommandList->SetGraphicsRootSignature(GeometryPassData.RootSignature.Get());
        CommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        const D3D12_VIEWPORT Viewport = SceneView.GetD3dViewport();
        CommandList->RSSetViewports(1, &Viewport);

        static const D3D12_RECT ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
        CommandList->RSSetScissorRects(1, &ScissorRect);

        CommandList->OMSetRenderTargets(
            GBuffer.TEXTURES_COUNT,
            RtvHandles.data(),
            false,
            &DsvHandle
        );

        CommandList->SetDescriptorHeaps(1, GeometryPassData.GpuDescriptorHeap.GetAddressOf());

        return true;
    };

    ResetCommandList();

    for (auto& RtvHandle: RtvHandles)
    {
        constexpr static FLOAT ClearColor[] = {0.f, 0.f, 0.f, 1.f};
        CommandList->ClearRenderTargetView(RtvHandle, ClearColor, 0, nullptr);
    }

    CommandList->ClearDepthStencilView(
        GBuffer.CpuDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f,
        0,
        0,
        nullptr
    );

    int DrawIndex = -1;
    for (const std::shared_ptr<Core::TexturedMesh>& TexturedMesh : Scene->GetTexturedMeshes())
    {
        nvtx3::scoped_range MeshIterationRange {"Mesh Iteration"};

        auto Mesh = TexturedMesh->GetMesh();

        // Make sure we have enough descriptors to draw this textured mesh
        {
            if (DrawIndex + 1 == GeometryPassData.PARALLEL_DRAWS_COUNT_ALLOWED)
            {
                if (!ExecuteCommandList())
                    return false;

                WaitDirectQueue();

                DrawIndex = -1;

                if (!ResetCommandList())
                    return false;
            }
            DrawIndex += 1;
        }

        // Setup Mesh
        {
            D3D12_VERTEX_BUFFER_VIEW VertexBufferView = Mesh->GetVertexBufferView();
            CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);

            if (Mesh->IsUsingIndices())
            {
                D3D12_INDEX_BUFFER_VIEW IndexBufferView = Mesh->GetIndexBufferView();
                CommandList->IASetIndexBuffer(&IndexBufferView);
            }
            else
            {
                CommandList->IASetIndexBuffer(nullptr);
            }
        }

        // Prepare SRV descriptor heap
        {
            D3D12_CPU_DESCRIPTOR_HANDLE SourceHandles[GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT] = {
                TexturedMesh->HasTexture(GeometryPassData.DIFFUSE_TEXTURE_NAME)
                    ? TexturedMesh->GetTexture(GeometryPassData.DIFFUSE_TEXTURE_NAME)->GetTextureHandle()
                    : CpuEmptyTextureHeap->GetCPUDescriptorHandleForHeapStart(),

                TexturedMesh->HasTexture(GeometryPassData.METALLIC_TEXTURE_NAME)
                    ? TexturedMesh->GetTexture(GeometryPassData.METALLIC_TEXTURE_NAME)->GetTextureHandle()
                    : CpuEmptyTextureHeap->GetCPUDescriptorHandleForHeapStart(),

                TexturedMesh->HasTexture(GeometryPassData.ROUGHNESS_TEXTURE_NAME)
                    ? TexturedMesh->GetTexture(GeometryPassData.ROUGHNESS_TEXTURE_NAME)->GetTextureHandle()
                    : CpuEmptyTextureHeap->GetCPUDescriptorHandleForHeapStart(),

                TexturedMesh->HasTexture(GeometryPassData.NORMAL_TEXTURE_NAME)
                    ? TexturedMesh->GetTexture(GeometryPassData.NORMAL_TEXTURE_NAME)->GetTextureHandle()
                    : CpuEmptyTextureHeap->GetCPUDescriptorHandleForHeapStart(),

                TexturedMesh->HasTexture(GeometryPassData.EMISSIVE_TEXTURE_NAME)
                    ? TexturedMesh->GetTexture(GeometryPassData.EMISSIVE_TEXTURE_NAME)->GetTextureHandle()
                    : CpuEmptyTextureHeap->GetCPUDescriptorHandleForHeapStart(),
            };

            D3D12_CPU_DESCRIPTOR_HANDLE DestinationHandles[GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT] = {};
            for (int i = 0; i < GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT; ++i)
            {
                DestinationHandles[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE {
                    GeometryPassData.GpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                    DrawIndex * static_cast<int>(GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT) + i,
                    SrvHandleIncrement
                };
            }

            UINT RangeSize[GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT] = {
                1,
                1,
                1,
                1,
                1
            };

            RenderApi->GetDevice()
                ->CopyDescriptors(
                    GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT,
                    DestinationHandles,
                    RangeSize,
                    GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT,
                    SourceHandles,
                    RangeSize,
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                );
        }

        // Set root signature parameters
        {
            const CD3DX12_GPU_DESCRIPTOR_HANDLE TableHandle {
                GeometryPassData.GpuDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
                DrawIndex * static_cast<int>(GeometryPassData.TEXTURED_MESH_TEXTURES_COUNT),
                SrvHandleIncrement
            };

            CommandList->SetGraphicsRootDescriptorTable(0, TableHandle);
            CommandList->SetGraphicsRootConstantBufferView(1, FrameData.ConstantBuffer->GetGPUVirtualAddress());
            CommandList->SetGraphicsRootConstantBufferView(2, TexturedMesh->GetConstantBufferGpuAddress());
        }

        // Draw
        {
            const std::int32_t PrimitivesCount = Mesh->GetPrimitivesCount();
            if (Mesh->IsUsingIndices())
            {
                CommandList->DrawIndexedInstanced(
                    PrimitivesCount,
                    1,
                    0,
                    0,
                    0
                );
            }
            else
            {
                CommandList->DrawInstanced(
                    PrimitivesCount,
                    1,
                    0,
                    0
                );
            }
        }
    }

    if (DrawIndex >= 0)
        if (!ExecuteCommandList())
            return false;

    return true;
}

bool DeferredRenderer::InitLightPass()
{
    // Create command list and allocators
    {
        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&LightPassData.RenderTargetToReadTransitionAllocator)),
            "Failed to create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&LightPassData.ReadToRenderTargetTransitionAllocator)),
            "Failed to create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    LightPassData.RenderTargetToReadTransitionAllocator.Get(), nullptr, IID_PPV_ARGS(&LightPassData.TransitionCommandList)),
            "Failed to create command list"
        )

        CHECKED_S(LightPassData.TransitionCommandList->Close());
    }

    return true;
}

bool DeferredRenderer::PrepareLightPassData(const Core::SceneView& SceneView)
{
    const glm::ivec2 ViewportSize = SceneView.GetViewportSize();

    if (LightPassData.Size == SceneView.GetViewportSize())
        return true;

    LightPassData.Size = ViewportSize;

    // Create or resize color texture
    {
        const CD3DX12_HEAP_PROPERTIES HeapProperties {D3D12_HEAP_TYPE_DEFAULT};
        const CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            LightPassData.COLOR_TEXTURE_FORMAT,
            ViewportSize.x,
            ViewportSize.y,
            1,
            1,
            1,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );

        const D3D12_CLEAR_VALUE ClearValue = {
            .Format = LightPassData.COLOR_TEXTURE_FORMAT,
            .Color = {
                0.f, 0.f, 0.f, 1.f
            }
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommittedResource(
                    &HeapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &ResourceDesc,
                    D3D12_RESOURCE_STATE_RENDER_TARGET,
                    &ClearValue,
                    IID_PPV_ARGS(&LightPassData.ColorTexture)
                ),
            "Can't create light pass color texture"
        )
    }

    // Create RTV heap and handle
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = 1,
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&LightPassData.CpuRtvHeap)),
            "Failed to create cpu rtv descriptor heap"
        )

        RenderApi->GetDevice()
            ->CreateRenderTargetView(LightPassData.ColorTexture.Get(), nullptr, LightPassData.CpuRtvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Create SRV heap and handle
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = 1,
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&LightPassData.CpuSrvHeap)),
            "Failed to create cpu srv descriptor heap"
        )

        RenderApi->GetDevice()
            ->CreateShaderResourceView(LightPassData.ColorTexture.Get(), nullptr, LightPassData.CpuSrvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    return true;
}

bool DeferredRenderer::TransitionLightPassFromRenderTargetToReadState()
{
    CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(LightPassData.ColorTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

    CHECKED_S(LightPassData.RenderTargetToReadTransitionAllocator->Reset());
    CHECKED_S(LightPassData.TransitionCommandList->Reset(LightPassData.RenderTargetToReadTransitionAllocator.Get(), nullptr))

    LightPassData.TransitionCommandList->ResourceBarrier(1, &Barrier);
    CHECKED_S(LightPassData.TransitionCommandList->Close());

    ID3D12CommandList* CommandLists[] = {LightPassData.TransitionCommandList.Get()};
    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    return true;
}

bool DeferredRenderer::TransitionLightPassFromReadToRenderTargetState()
{
    CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(LightPassData.ColorTexture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

    CHECKED_S(LightPassData.TransitionCommandList->Reset(LightPassData.RenderTargetToReadTransitionAllocator.Get(), nullptr))

    LightPassData.TransitionCommandList->ResourceBarrier(1, &Barrier);
    CHECKED_S(LightPassData.TransitionCommandList->Close());

    ID3D12CommandList* CommandLists[] = {LightPassData.TransitionCommandList.Get()};
    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    return true;
}

bool DeferredRenderer::InitAmbientDirectionalLightPass()
{
    // Create Root
    {
        CD3DX12_ROOT_PARAMETER RootParams[2] {};

        // Descriptor table with GBuffer textures
        CD3DX12_DESCRIPTOR_RANGE GBufferTexturesRange {};
        GBufferTexturesRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBuffer.TEXTURES_COUNT, 0);

        RootParams[0].InitAsDescriptorTable(1, &GBufferTexturesRange);

        // Frame constant buffer
        RootParams[1].InitAsConstantBufferView(0);

        const auto& StaticSamplers = GetCommonStaticSamplers();

        CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc {};
        RootSignatureDesc.Init(
            std::size(RootParams),
            RootParams,
            StaticSamplers.size(),
            StaticSamplers.data(),
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureBlob {};
        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrorBlob {};

        HRESULT RootSigSerResult = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &RootSignatureBlob, &RootSignatureErrorBlob);
        if (FAILED(RootSigSerResult))
        {
            std::string error (static_cast<const char*>(RootSignatureErrorBlob->GetBufferPointer()), RootSignatureErrorBlob->GetBufferSize());
            __debugbreak();
            return false;
        }

        CHECKED(
            RenderApi->GetDevice()
                ->CreateRootSignature(
                    0,
                    RootSignatureBlob->GetBufferPointer(),
                    RootSignatureBlob->GetBufferSize(),
                    IID_PPV_ARGS(&AmbientDirectionalLightPassData.RootSignature)
                ),
            "Can't create root signature"
        )
    }

    // Load Shaders
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader {};
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader {};
    {
        Microsoft::WRL::ComPtr<ID3DBlob> CompilationErrorBlob {};

        RenderApi::Core::ContentFolderD3dInclude VertexShaderInclude {};

        HRESULT VSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/AmbientDirectionalLightPass.hlsl",
            nullptr, &VertexShaderInclude, "VS_Main", "vs_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &VertexShader, &CompilationErrorBlob);

        if(FAILED(VSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }

        RenderApi::Core::ContentFolderD3dInclude PixelShaderInclude {};

        HRESULT PSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/AmbientDirectionalLightPass.hlsl",
            nullptr, &PixelShaderInclude, "PS_Main", "ps_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &PixelShader, &CompilationErrorBlob);

        if(FAILED(PSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }
    }


    // Create PSO
    {
        auto RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        RasterizerState.FrontCounterClockwise = true;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc {
            .pRootSignature = AmbientDirectionalLightPassData.RootSignature.Get(),
            .VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get()),
            .PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get()),
            .BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            .SampleMask = UINT_MAX,
            .RasterizerState = RasterizerState,
            .InputLayout = {
                .pInputElementDescs = RenderApi->GetCommonMeshBufferLayout().Layout.data(),
                .NumElements = static_cast<UINT>(RenderApi->GetCommonMeshBufferLayout().Layout.size())
            },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = 1,
            .RTVFormats = {
                DXGI_FORMAT_R16G16B16A16_FLOAT,
            },
            .SampleDesc = {
                .Count = 1
            }
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&AmbientDirectionalLightPassData.PipelineState)),
            "Failed to create PSO"
        )

        AmbientDirectionalLightPassData.PipelineState->SetName(L"Ambient Directional Pass PSO");
    }

    // Create command list and allocator
    {
        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&AmbientDirectionalLightPassData.CommandAllocator)),
            "Failed to create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    AmbientDirectionalLightPassData.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&AmbientDirectionalLightPassData.CommandList)),
            "Failed to create command list"
        )

        CHECKED_S(AmbientDirectionalLightPassData.CommandList->Close());
    }

    // Create GPU SRV heap
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = GBuffer.TEXTURES_COUNT,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&AmbientDirectionalLightPassData.GpuDescriptorHeap)),
            "Failed to create descriptor heap"
        )
    }

    return true;
}

bool DeferredRenderer::AmbientDirectionalLightPass(const Core::SceneView& SceneView)
{
    nvtx3::scoped_range PassRange {"Ambient & Directional Light Pass"};

    auto& CommandList = AmbientDirectionalLightPassData.CommandList;
    auto& CommandAllocator = AmbientDirectionalLightPassData.CommandAllocator;

    CHECKED_S(CommandAllocator->Reset());
    CHECKED_S(CommandList->Reset(CommandAllocator.Get(), AmbientDirectionalLightPassData.PipelineState.Get()));

    // Copy GBuffer descriptors
    {
        RenderApi->GetDevice()
            ->CopyDescriptorsSimple(
                GBuffer.TEXTURES_COUNT,
                AmbientDirectionalLightPassData.GpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                GBuffer.CpuSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
            );
    }

    // Setup Render target
    {
        constexpr static FLOAT ClearColor[4] = {0.f, 0.f, 0.f, 1.f};
        CommandList->ClearRenderTargetView(LightPassData.CpuRtvHeap->GetCPUDescriptorHandleForHeapStart(), ClearColor, 0, nullptr);

        const D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle = LightPassData.CpuRtvHeap->GetCPUDescriptorHandleForHeapStart();
        CommandList->OMSetRenderTargets(1, &RtvHandle, true, nullptr);

        D3D12_VIEWPORT Viewport = SceneView.GetD3dViewport();
        CommandList->RSSetViewports(1, &Viewport);

        static const D3D12_RECT ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
        CommandList->RSSetScissorRects(1, &ScissorRect);
    }

    // Setup Root Params
    {
        CommandList->SetGraphicsRootSignature(AmbientDirectionalLightPassData.RootSignature.Get());

        CommandList->SetDescriptorHeaps(1, AmbientDirectionalLightPassData.GpuDescriptorHeap.GetAddressOf());
        CommandList->SetGraphicsRootDescriptorTable(0, AmbientDirectionalLightPassData.GpuDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

        CommandList->SetGraphicsRootConstantBufferView(1, FrameData.ConstantBuffer->GetGPUVirtualAddress());
    }

    // Setup fullscreen quad mesh
    {
        const D3D12_VERTEX_BUFFER_VIEW VertexBufferView = FullscreenQuadMesh->GetVertexBufferView();
        CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);

        if (FullscreenQuadMesh->IsUsingIndices())
        {
            const D3D12_INDEX_BUFFER_VIEW IndexBufferView = FullscreenQuadMesh->GetIndexBufferView();
            CommandList->IASetIndexBuffer(&IndexBufferView);
        }

        CommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // Draw
    {
        if (FullscreenQuadMesh->IsUsingIndices())
        {
            CommandList->DrawIndexedInstanced(FullscreenQuadMesh->GetPrimitivesCount(), 1, 0, 0, 0);
        }
        else
        {
            CommandList->DrawInstanced(FullscreenQuadMesh->GetPrimitivesCount(), 1, 0, 0);
        }
    }

    CHECKED_S(CommandList->Close());

    ID3D12CommandList* CommandLists[] = {CommandList.Get()};
    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    return true;
}

bool DeferredRenderer::InitPointLightShadowCubeMapPass()
{
    // Create Root
    {
        CD3DX12_ROOT_PARAMETER RootParams[4] {};

        // Frame constant buffer
        RootParams[0].InitAsConstantBufferView(0);

        // Point light constant buffer
        RootParams[1].InitAsConstantBufferView(1);

        // Mesh constant buffer
        RootParams[2].InitAsConstantBufferView(2);

        // View Projection matrix for cube map face
        RootParams[3].InitAsConstants(16, 3);

        const auto& StaticSamplers = GetCommonStaticSamplers();

        CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc {};
        RootSignatureDesc.Init(
            std::size(RootParams),
            RootParams,
            StaticSamplers.size(),
            StaticSamplers.data(),
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureBlob {};
        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrorBlob {};

        HRESULT RootSigSerResult = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &RootSignatureBlob, &RootSignatureErrorBlob);
        if (FAILED(RootSigSerResult))
        {
            std::string error (static_cast<const char*>(RootSignatureErrorBlob->GetBufferPointer()), RootSignatureErrorBlob->GetBufferSize());
            __debugbreak();
            return false;
        }

        CHECKED(
            RenderApi->GetDevice()
                ->CreateRootSignature(
                    0,
                    RootSignatureBlob->GetBufferPointer(),
                    RootSignatureBlob->GetBufferSize(),
                    IID_PPV_ARGS(&PointLightShadowCubeMapData.RootSignature)
                ),
            "Can't create root signature"
        )
    }

    // Load Shaders
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader {};
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader {};
    {
        Microsoft::WRL::ComPtr<ID3DBlob> CompilationErrorBlob {};

        RenderApi::Core::ContentFolderD3dInclude VertexShaderInclude {};

        HRESULT VSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/PointLight/PointLightShadowCubemapPass.hlsl",
            nullptr, &VertexShaderInclude, "VS_Main", "vs_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &VertexShader, &CompilationErrorBlob);

        if(FAILED(VSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }

        RenderApi::Core::ContentFolderD3dInclude PixelShaderInclude {};

        HRESULT PSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/PointLight/PointLightShadowCubemapPass.hlsl",
            nullptr, &PixelShaderInclude, "PS_Main", "ps_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &PixelShader, &CompilationErrorBlob);

        if(FAILED(PSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }
    }

    // Create PSO
    {
        auto RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc {
            .pRootSignature = PointLightShadowCubeMapData.RootSignature.Get(),
            .VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get()),
            .PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get()),
            .BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            .SampleMask = UINT_MAX,
            .RasterizerState = RasterizerState,
            .DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
            .InputLayout = {
                .pInputElementDescs = RenderApi->GetCommonMeshBufferLayout().Layout.data(),
                .NumElements = static_cast<UINT>(RenderApi->GetCommonMeshBufferLayout().Layout.size())
            },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = 1,
            .RTVFormats = {
                Core::PointLight::CUBE_MAP_FORMAT,
            },
            .DSVFormat = Core::PointLight::CUBE_MAP_DEPTH_STENCIL_FORMAT,
            .SampleDesc = {
                .Count = 1
            }
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PointLightShadowCubeMapData.PipelineState)),
            "Failed to create PSO"
        )

        PointLightShadowCubeMapData.PipelineState->SetName(L"Point Light Shadow Cube Map Pass PSO");
    }

    // Create command list and allocator
    {
        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&PointLightShadowCubeMapData.CommandAllocator)),
            "Failed to create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    PointLightShadowCubeMapData.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&PointLightShadowCubeMapData.CommandList)),
            "Failed to create command list"
        )

        CHECKED_S(PointLightShadowCubeMapData.CommandList->Close());
    }

    return true;
}

bool DeferredRenderer::PointLightShadowCubeMapsPass()
{
    nvtx3::scoped_range PassRange {"Point Light Shadow Cube Maps Pass"};

    auto& CommandAllocator = PointLightShadowCubeMapData.CommandAllocator;
    auto& CommandList = PointLightShadowCubeMapData.CommandList;

    CHECKED_S(CommandAllocator->Reset());
    CHECKED_S(CommandList->Reset(CommandAllocator.Get(), PointLightShadowCubeMapData.PipelineState.Get()));

    // Transition to present state
    {
        for(const auto& PointLight : Scene->GetPointLights())
        {
            if (!PointLight->CastsShadows())
                continue;

            PointLight->TransitionShadowCubeMapFromReadToRenderTarget(CommandList.Get());
        }
    }

    // Prepare command list for rendering
    {
        CommandList->SetGraphicsRootSignature(PointLightShadowCubeMapData.RootSignature.Get());
        CommandList->SetGraphicsRootConstantBufferView(0, FrameData.ConstantBuffer->GetGPUVirtualAddress());

        const D3D12_VIEWPORT Viewport = CD3DX12_VIEWPORT(
            0.f,
            0.f,
            Core::PointLight::SHADOW_MAP_SIZE,
            Core::PointLight::SHADOW_MAP_SIZE
        );
        CommandList->RSSetViewports(1, &Viewport);

        const CD3DX12_RECT ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
        CommandList->RSSetScissorRects(1, &ScissorRect);

        CommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // Render shadow maps
    {
        for(const auto& PointLight : Scene->GetPointLights())
        {
            if (!PointLight->CastsShadows())
                continue;

            CommandList->SetGraphicsRootConstantBufferView(1, PointLight->GetConstantBufferGpuHandle());

            glm::mat4 ProjectionMatrix = glm::perspective(
                glm::radians(90.f),
                1.0f,
                1.0f,
                PointLight->GetShadowFarDistance()
            );

            const std::array ShadowViewMatrices = {
                glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{1, 0, 0}, {0, 1, 0}),
                glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{-1, 0, 0}, {0, 1, 0}),
                glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{0, 1, 0}, {0, 0, -1}),
                glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{0, -1, 0}, {0, 0, 1}),
                glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{0, 0, 1}, {0, 1, 0}),
                glm::lookAt(PointLight->GetPosition(), PointLight->GetPosition() + glm::vec3{0, 0, -1}, {0, 1, 0}),
            };

            // Render each side of point light
            for (int i = 0; i < 6; ++i)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle = PointLight->GetShadowCubeMapRtvHandle(i);
                D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = PointLight->GetShadowCubeMapDsvHandle(i);
                CommandList->OMSetRenderTargets(1, &RtvHandle, true, &DsvHandle);

                FLOAT ClearColor[4] = {1.f, 1.f, 1.f, 1.f};
                CommandList->ClearRenderTargetView(RtvHandle, ClearColor, 0, nullptr);

                CommandList->ClearDepthStencilView(DsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

                glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ShadowViewMatrices[i];

                CommandList->SetGraphicsRoot32BitConstants(3, 16, &ViewProjectionMatrix[0][0], 0);

                for (const auto& TexturedMesh : Scene->GetTexturedMeshes())
                {
                    auto Mesh = TexturedMesh->GetMesh();

                    CommandList->SetGraphicsRootConstantBufferView(2, TexturedMesh->GetConstantBufferGpuAddress());

                    // Set up mesh
                    {
                        D3D12_VERTEX_BUFFER_VIEW VertexBufferView = Mesh->GetVertexBufferView();
                        CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);

                        if (Mesh->IsUsingIndices())
                        {
                            D3D12_INDEX_BUFFER_VIEW IndexBufferView = Mesh->GetIndexBufferView();
                            CommandList->IASetIndexBuffer(&IndexBufferView);
                        }
                        else
                        {
                            CommandList->IASetIndexBuffer(nullptr);
                        }
                    }

                    // Draw
                    {
                        if (Mesh->IsUsingIndices())
                        {
                            CommandList->DrawIndexedInstanced(Mesh->GetPrimitivesCount(), 1, 0, 0, 0);
                        }
                        else
                        {
                            CommandList->DrawInstanced(Mesh->GetPrimitivesCount(), 1, 0, 0);
                        }
                    }
                }
            }
        }
    }

    // Transition to back read state
    {
        for(const auto& PointLight : Scene->GetPointLights())
        {
            if (!PointLight->CastsShadows())
                continue;

            PointLight->TransitionShadowCubeMapFromRenderTargetToRead(CommandList.Get());
        }
    }

    CHECKED_S(CommandList->Close());

    ID3D12CommandList* CommandLists[] = {CommandList.Get()};
    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    return true;
}

bool DeferredRenderer::InitPointLightVolumePass()
{
    // Create Root
    {
        CD3DX12_ROOT_PARAMETER RootParams[4] {};

        // Descriptor table with GBuffer textures
        CD3DX12_DESCRIPTOR_RANGE GBufferAndLightPassTexturesRange {};
        GBufferAndLightPassTexturesRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBuffer.TEXTURES_COUNT, 0);

        RootParams[0].InitAsDescriptorTable(1, &GBufferAndLightPassTexturesRange);

        // Frame constant buffer
        RootParams[1].InitAsConstantBufferView(0);

        // Point Light constant buffer
        RootParams[2].InitAsConstantBufferView(1);

        // Shadow Cube Map
        CD3DX12_DESCRIPTOR_RANGE ShadowCubeMapRange {};
        ShadowCubeMapRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);

        RootParams[3].InitAsDescriptorTable(1, &ShadowCubeMapRange);

        const auto& StaticSamplers = GetCommonStaticSamplers();

        CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc {};
        RootSignatureDesc.Init(
            std::size(RootParams),
            RootParams,
            StaticSamplers.size(),
            StaticSamplers.data(),
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureBlob {};
        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrorBlob {};

        HRESULT RootSigSerResult = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &RootSignatureBlob, &RootSignatureErrorBlob);
        if (FAILED(RootSigSerResult))
        {
            std::string error (static_cast<const char*>(RootSignatureErrorBlob->GetBufferPointer()), RootSignatureErrorBlob->GetBufferSize());
            __debugbreak();
            return false;
        }

        CHECKED(
            RenderApi->GetDevice()
                ->CreateRootSignature(
                    0,
                    RootSignatureBlob->GetBufferPointer(),
                    RootSignatureBlob->GetBufferSize(),
                    IID_PPV_ARGS(&PointLightVolumePassData.RootSignature)
                ),
            "Can't create root signature"
        )
    }

    // Load Shaders
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader {};
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader {};
    Microsoft::WRL::ComPtr<ID3DBlob> EmptyPixelShader {};
    {
        Microsoft::WRL::ComPtr<ID3DBlob> CompilationErrorBlob {};

        RenderApi::Core::ContentFolderD3dInclude VertexShaderInclude {};

        HRESULT VSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/PointLight/PointLightVolumePass.hlsl",
            nullptr, &VertexShaderInclude, "VS_Main", "vs_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &VertexShader, &CompilationErrorBlob);

        if(FAILED(VSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }

        RenderApi::Core::ContentFolderD3dInclude PixelShaderInclude {};

        HRESULT PSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/PointLight/PointLightVolumePass.hlsl",
            nullptr, &PixelShaderInclude, "PS_Main", "ps_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &PixelShader, &CompilationErrorBlob);

        if(FAILED(PSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }

        RenderApi::Core::ContentFolderD3dInclude EmptyPixelShaderInclude {};

        HRESULT EmptyPSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/PointLight/EmptyPS.hlsl",
            nullptr, &EmptyPixelShaderInclude, "PS_Main", "ps_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &EmptyPixelShader, &CompilationErrorBlob);

        if(FAILED(PSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }
    }

    // Create PSO for depth stencil part
    {
        auto RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        RasterizerState.FrontCounterClockwise = true;
        RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

        auto DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        DepthStencilState.StencilEnable = true;
        DepthStencilState.StencilWriteMask = 0xff;
        DepthStencilState.StencilReadMask = 0x00;
        DepthStencilState.BackFace = {
            .StencilFailOp = D3D12_STENCIL_OP_KEEP,
            .StencilDepthFailOp = D3D12_STENCIL_OP_INCR,
            .StencilPassOp = D3D12_STENCIL_OP_KEEP,
            .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS
        };
        DepthStencilState.FrontFace = {
            .StencilFailOp = D3D12_STENCIL_OP_KEEP,
            .StencilDepthFailOp = D3D12_STENCIL_OP_DECR,
            .StencilPassOp = D3D12_STENCIL_OP_KEEP,
            .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS
        };

        auto BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        BlendState.RenderTarget[0].RenderTargetWriteMask = 0x00;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc {
            .pRootSignature = PointLightVolumePassData.RootSignature.Get(),
            .VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get()),
            .PS = CD3DX12_SHADER_BYTECODE(EmptyPixelShader.Get()),
            .BlendState = BlendState,
            .SampleMask = UINT_MAX,
            .RasterizerState = RasterizerState,
            .DepthStencilState = DepthStencilState,
            .InputLayout = {
                .pInputElementDescs = RenderApi->GetCommonMeshBufferLayout().Layout.data(),
                .NumElements = static_cast<UINT>(RenderApi->GetCommonMeshBufferLayout().Layout.size())
            },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = 0,
            .RTVFormats = {},
            .DSVFormat = Core::PointLight::DEPTH_STENCIL_FORMAT,
            .SampleDesc = {
                .Count = 1
            }
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PointLightVolumePassData.StencilPipelineState)),
            "Failed to create PSO"
        )

        PointLightVolumePassData.StencilPipelineState->SetName(L"Point Light Stencil PSO");
    }

    // Create PSO for color part
    {
        auto RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        RasterizerState.FrontCounterClockwise = true;
        RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

        auto DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        DepthStencilState.DepthEnable = false;
        DepthStencilState.StencilEnable = true;
        DepthStencilState.StencilWriteMask = 0x00;
        DepthStencilState.StencilReadMask = 0xff;
        DepthStencilState.BackFace = {
            .StencilFailOp = D3D12_STENCIL_OP_KEEP,
            .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
            .StencilPassOp = D3D12_STENCIL_OP_KEEP,
            .StencilFunc = D3D12_COMPARISON_FUNC_EQUAL
        };

        auto BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        BlendState.RenderTarget[0] = {
            .BlendEnable = true,
            .LogicOpEnable = false,
            .SrcBlend = D3D12_BLEND_ONE,
            .DestBlend = D3D12_BLEND_ONE,
            .BlendOp = D3D12_BLEND_OP_ADD,
            .SrcBlendAlpha = D3D12_BLEND_ONE,
            .DestBlendAlpha = D3D12_BLEND_ONE,
            .BlendOpAlpha = D3D12_BLEND_OP_MAX,
            .LogicOp = D3D12_LOGIC_OP_NOOP,
            .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc {
            .pRootSignature = PointLightVolumePassData.RootSignature.Get(),
            .VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get()),
            .PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get()),
            .BlendState = BlendState,
            .SampleMask = UINT_MAX,
            .RasterizerState = RasterizerState,
            .DepthStencilState = DepthStencilState,
            .InputLayout = {
                .pInputElementDescs = RenderApi->GetCommonMeshBufferLayout().Layout.data(),
                .NumElements = static_cast<UINT>(RenderApi->GetCommonMeshBufferLayout().Layout.size())
            },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = 1,
            .RTVFormats = {
                DXGI_FORMAT_R16G16B16A16_FLOAT,
            },
            .DSVFormat = Core::PointLight::DEPTH_STENCIL_FORMAT,
            .SampleDesc = {
                .Count = 1
            }
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PointLightVolumePassData.ColorPipelineState)),
            "Failed to create PSO"
        )

        PointLightVolumePassData.ColorPipelineState->SetName(L"Point Light Color PSO");
    }

    // Create command list and allocator
    {
        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&PointLightVolumePassData.CommandAllocator)),
            "Failed to create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    PointLightVolumePassData.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&PointLightVolumePassData.CommandList)),
            "Failed to create command list"
        )

        CHECKED_S(PointLightVolumePassData.CommandList->Close());
    }

    // Create GPU descriptors heap
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = GBuffer.TEXTURES_COUNT + PointLightVolumePassData.MAX_DYNAMIC_POINT_LIGHTS_COUNT,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&PointLightVolumePassData.GpuDescriptorHeap)),
            "Failed to create descriptor heap"
        )
    }

    return true;
}

bool DeferredRenderer::PointLightVolumesPass(const Core::SceneView& SceneView)
{
    nvtx3::scoped_range PassRange {"Point Light Volumes Pass"};

    auto& CommandAllocator = PointLightVolumePassData.CommandAllocator;
    auto& CommandList = PointLightVolumePassData.CommandList;

    CHECKED_S(CommandAllocator->Reset());
    CHECKED_S(CommandList->Reset(CommandAllocator.Get(), PointLightVolumePassData.StencilPipelineState.Get()));

    // Fill GBuffer descriptors
    {
        RenderApi->GetDevice()
            ->CopyDescriptorsSimple(
                GBuffer.TEXTURES_COUNT,
                PointLightVolumePassData.GpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                GBuffer.CpuSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
            );
    }

    // Prepare point light depth stencil buffers
    {
        CD3DX12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                GBuffer.DepthStencilTexture.Get(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );

        CommandList->ResourceBarrier(1, &ResourceBarrier);

        for (const auto& PointLight : Scene->GetPointLights())
        {
            PointLight->PrepareDepthStencilForVolumeRendering(
                *RenderApi,
                SceneView.GetViewportSize(),
                CommandList.Get(),
                GBuffer.DepthStencilTexture.Get()
            );
        }

        ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                GBuffer.DepthStencilTexture.Get(),
                D3D12_RESOURCE_STATE_COPY_SOURCE,
                D3D12_RESOURCE_STATE_DEPTH_WRITE
            );

        CommandList->ResourceBarrier(1, &ResourceBarrier);
    }

    // Set up command list
    {
        CommandList->SetGraphicsRootSignature(PointLightVolumePassData.RootSignature.Get());
        CommandList->SetDescriptorHeaps(1, PointLightVolumePassData.GpuDescriptorHeap.GetAddressOf());

        CommandList->SetGraphicsRootDescriptorTable(0, PointLightVolumePassData.GpuDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
        CommandList->SetGraphicsRootConstantBufferView(1, FrameData.ConstantBuffer->GetGPUVirtualAddress());

        D3D12_VERTEX_BUFFER_VIEW VertexBufferView = SphereMesh->GetVertexBufferView();
        CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);

        if (SphereMesh->IsUsingIndices())
        {
            D3D12_INDEX_BUFFER_VIEW IndexBufferView = SphereMesh->GetIndexBufferView();
            CommandList->IASetIndexBuffer(&IndexBufferView);
        }

        CommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        D3D12_VIEWPORT Viewport = SceneView.GetD3dViewport();
        CommandList->RSSetViewports(1, &Viewport);

        D3D12_RECT Scissors = D3D12_RECT(0, 0, LONG_MAX, LONG_MAX);
        CommandList->RSSetScissorRects(1, &Scissors);
    }

    // Fill stencil buffers
    {
        for (const auto& PointLight : Scene->GetPointLights())
        {
            CommandList->SetGraphicsRootConstantBufferView(2, PointLight->GetConstantBufferGpuHandle());

            D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = PointLight->GetDepthStencilVolumeDsvHandle();
            CommandList->OMSetRenderTargets(0, nullptr, false, &DsvHandle);

            if (SphereMesh->IsUsingIndices())
            {
                CommandList->DrawIndexedInstanced(SphereMesh->GetPrimitivesCount(), 1, 0, 0, 0);
            }
            else
            {
                CommandList->DrawInstanced(SphereMesh->GetPrimitivesCount(), 1, 0, 0);
            }
        }
    }

    CommandList->SetPipelineState(PointLightVolumePassData.ColorPipelineState.Get());
    CommandList->OMSetStencilRef(1);

    // Draw color based on stencil buffers
    {
        int PointLightIndex = 0;

        for (const auto& PointLight : Scene->GetPointLights())
        {
            CommandList->SetGraphicsRootConstantBufferView(2, PointLight->GetConstantBufferGpuHandle());

            const unsigned SrvIncrement = RenderApi->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            CD3DX12_CPU_DESCRIPTOR_HANDLE CpuShadowCubeMapSrvHandle {
                PointLightVolumePassData.GpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                static_cast<int>(GBuffer.TEXTURES_COUNT) + PointLightIndex,
                SrvIncrement
            };

            RenderApi->GetDevice()
                ->CopyDescriptorsSimple(
                    1,
                    CpuShadowCubeMapSrvHandle,
                    PointLight->GetShadowCubeMapSrvHandle(),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                );

            CD3DX12_GPU_DESCRIPTOR_HANDLE GpuShadowCubeMapSrvHandle {
                PointLightVolumePassData.GpuDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
                static_cast<int>(GBuffer.TEXTURES_COUNT) + PointLightIndex,
                SrvIncrement
            };

            CommandList->SetGraphicsRootDescriptorTable(3, GpuShadowCubeMapSrvHandle);

            D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle = LightPassData.CpuRtvHeap->GetCPUDescriptorHandleForHeapStart();
            D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = PointLight->GetDepthStencilVolumeDsvHandle();
            CommandList->OMSetRenderTargets(1, &RtvHandle, true, &DsvHandle);

            if (SphereMesh->IsUsingIndices())
            {
                CommandList->DrawIndexedInstanced(SphereMesh->GetPrimitivesCount(), 1, 0, 0, 0);
            }
            else
            {
                CommandList->DrawInstanced(SphereMesh->GetPrimitivesCount(), 1, 0, 0);
            }

            ++PointLightIndex;
        }
    }

    CHECKED_S(CommandList->Close())

    ID3D12CommandList* CommandLists[] = {CommandList.Get()};
    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    return true;
}

bool DeferredRenderer::InitPostProcessingPass()
{
    // Create Root
    {
        CD3DX12_ROOT_PARAMETER RootParams[2] {};

        // Descriptor table with GBuffer textures
        CD3DX12_DESCRIPTOR_RANGE GBufferAndLightPassTexturesRange {};
        GBufferAndLightPassTexturesRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GBuffer.TEXTURES_COUNT + 1, 0);

        RootParams[0].InitAsDescriptorTable(1, &GBufferAndLightPassTexturesRange);

        // Frame constant buffer
        RootParams[1].InitAsConstantBufferView(0);

        const auto& StaticSamplers = GetCommonStaticSamplers();

        CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc {};
        RootSignatureDesc.Init(
            std::size(RootParams),
            RootParams,
            StaticSamplers.size(),
            StaticSamplers.data(),
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureBlob {};
        Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrorBlob {};

        HRESULT RootSigSerResult = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &RootSignatureBlob, &RootSignatureErrorBlob);
        if (FAILED(RootSigSerResult))
        {
            std::string error (static_cast<const char*>(RootSignatureErrorBlob->GetBufferPointer()), RootSignatureErrorBlob->GetBufferSize());
            __debugbreak();
            return false;
        }

        CHECKED(
            RenderApi->GetDevice()
                ->CreateRootSignature(
                    0,
                    RootSignatureBlob->GetBufferPointer(),
                    RootSignatureBlob->GetBufferSize(),
                    IID_PPV_ARGS(&PostProcessingPassData.RootSignature)
                ),
            "Can't create root signature"
        )
    }

    // Load Shaders
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader {};
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader {};
    {
        Microsoft::WRL::ComPtr<ID3DBlob> CompilationErrorBlob {};

        RenderApi::Core::ContentFolderD3dInclude VertexShaderInclude {};

        HRESULT VSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/PostProcessingPass.hlsl",
            nullptr, &VertexShaderInclude, "VS_Main", "vs_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &VertexShader, &CompilationErrorBlob);

        if(FAILED(VSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }

        RenderApi::Core::ContentFolderD3dInclude PixelShaderInclude {};

        HRESULT PSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_renderer_deferred/Shaders/Passes/PostProcessingPass.hlsl",
            nullptr, &PixelShaderInclude, "PS_Main", "ps_5_1",
            RenderApi->GetShaderCompileFlags(), 0, &PixelShader, &CompilationErrorBlob);

        if(FAILED(PSCompileResult) || CompilationErrorBlob != nullptr)
        {
            std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
            // TODO: log error

            __debugbreak();

            return false;
        }
    }

    // Create PSO
    {
        auto RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        RasterizerState.FrontCounterClockwise = true;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc {
            .pRootSignature = PostProcessingPassData.RootSignature.Get(),
            .VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get()),
            .PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get()),
            .BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            .SampleMask = UINT_MAX,
            .RasterizerState = RasterizerState,
            .InputLayout = {
                .pInputElementDescs = RenderApi->GetCommonMeshBufferLayout().Layout.data(),
                .NumElements = static_cast<UINT>(RenderApi->GetCommonMeshBufferLayout().Layout.size())
            },
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = 1,
            .RTVFormats = {
                DXGI_FORMAT_R8G8B8A8_UNORM,
            },
            .SampleDesc = {
                .Count = 1
            }
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PostProcessingPassData.PipelineState)),
            "Failed to create PSO"
        )

        PostProcessingPassData.PipelineState->SetName(L"Post Processing Pass PSO");
    }

    // Create command list and allocator
    {
        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&PostProcessingPassData.CommandAllocator)),
            "Failed to create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    PostProcessingPassData.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&PostProcessingPassData.CommandList)),
            "Failed to create command list"
        )

        CHECKED_S(PostProcessingPassData.CommandList->Close());
    }

    // Create GPU SRV heap
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = GBuffer.TEXTURES_COUNT + 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&PostProcessingPassData.GpuDescriptorHeap)),
            "Failed to create descriptor heap"
        )
    }

    return true;
}

bool DeferredRenderer::PostProcessingPass(const Core::SceneView& SceneView)
{
    nvtx3::scoped_range PassRange {"Post Processing Pass"};

    auto& CommandList = PostProcessingPassData.CommandList;
    auto& CommandAllocator = PostProcessingPassData.CommandAllocator;

    CHECKED_S(CommandAllocator->Reset());
    CHECKED_S(CommandList->Reset(CommandAllocator.Get(), PostProcessingPassData.PipelineState.Get()));

    // Copy GBuffer descriptors
    {
        RenderApi->GetDevice()
            ->CopyDescriptorsSimple(
                GBuffer.TEXTURES_COUNT,
                PostProcessingPassData.GpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                GBuffer.CpuSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
            );

        RenderApi->GetDevice()
            ->CopyDescriptorsSimple(
                1,
                CD3DX12_CPU_DESCRIPTOR_HANDLE
                {
                    PostProcessingPassData.GpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                    GBuffer.TEXTURES_COUNT,
                    RenderApi->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
                },
                LightPassData.CpuSrvHeap->GetCPUDescriptorHandleForHeapStart(),
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
            );
    }

    // Setup Render target
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle = SceneView.GetRenderTargetHandle();

        constexpr static FLOAT ClearColor[4] = {0.f, 0.f, 0.f, 1.f};
        CommandList->ClearRenderTargetView(RtvHandle, ClearColor, 0, nullptr);

        CommandList->OMSetRenderTargets(1, &RtvHandle, true, nullptr);

        D3D12_VIEWPORT Viewport = SceneView.GetD3dViewport();
        CommandList->RSSetViewports(1, &Viewport);

        static const D3D12_RECT ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
        CommandList->RSSetScissorRects(1, &ScissorRect);
    }

    // Setup Root Params
    {
        CommandList->SetGraphicsRootSignature(PostProcessingPassData.RootSignature.Get());

        CommandList->SetDescriptorHeaps(1, PostProcessingPassData.GpuDescriptorHeap.GetAddressOf());
        CommandList->SetGraphicsRootDescriptorTable(0, PostProcessingPassData.GpuDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

        CommandList->SetGraphicsRootConstantBufferView(1, FrameData.ConstantBuffer->GetGPUVirtualAddress());
    }

    // Setup fullscreen quad mesh
    {
        const D3D12_VERTEX_BUFFER_VIEW VertexBufferView = FullscreenQuadMesh->GetVertexBufferView();
        CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);

        if (FullscreenQuadMesh->IsUsingIndices())
        {
            const D3D12_INDEX_BUFFER_VIEW IndexBufferView = FullscreenQuadMesh->GetIndexBufferView();
            CommandList->IASetIndexBuffer(&IndexBufferView);
        }

        CommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // Draw
    {
        if (FullscreenQuadMesh->IsUsingIndices())
        {
            CommandList->DrawIndexedInstanced(FullscreenQuadMesh->GetPrimitivesCount(), 1, 0, 0, 0);
        }
        else
        {
            CommandList->DrawInstanced(FullscreenQuadMesh->GetPrimitivesCount(), 1, 0, 0);
        }
    }

    CHECKED_S(CommandList->Close());

    ID3D12CommandList* CommandLists[] = {CommandList.Get()};
    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    return true;
}

bool DeferredRenderer::Shutdown()
{
    RenderThreadPool.Shutdown();

    return WaitDirectQueue();
}

bool DeferredRenderer::InitBasicMeshes()
{
    // Load fullscreen Quad

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> InitCommandAllocator {};
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> InitCommandList {};

    CHECKED(
        RenderApi->GetDevice()
        ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&InitCommandAllocator)),
        "Failed to create command allocator"
    )

    CHECKED(
        RenderApi->GetDevice()
        ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            InitCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&InitCommandList)),
        "Failed to create command list"
    )

    constexpr float QuadMesh[] = {
        -1.f, 1.f, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        -1.f, -1.f, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1.f, -1.f, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1.f, 1.f, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        -1.f, 1.f, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1.f, -1.f, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    FullscreenQuadMesh = std::make_shared<Core::Mesh>();
    const Core::Mesh::MeshLoadOperation MeshLoad = FullscreenQuadMesh->Load(*RenderApi.get(), *InitCommandList.Get(), RenderApi->ContainerToBytes(QuadMesh));

    if (!MeshLoad.WasSuccessful())
        return false;

    CHECKED_S(InitCommandList->Close());

    ID3D12CommandList* InitCommandLists[] = {InitCommandList.Get()};
    RenderApi->GetDirectQueue()
            ->ExecuteCommandLists(1, InitCommandLists);

    // Load unit sphere

    ModelLoader::LoadResult UnitSphereLoadResult = ModelLoader::LoadModel(
        "../Content/krendrr_runtime_renderer_deferred/UnitIcoSphere.obj",
        *RenderApi,
        {
            .ThreadPool = &RenderThreadPool
        }
    );

    if (!UnitSphereLoadResult.HasLoadedAtLeastOne())
    {
        // TODO: log error can't load unit sphere model
        return false;
    }

    SphereMesh = UnitSphereLoadResult.TexturedMeshes[0]->GetMesh();

    // Wait before removing mesh upload buffers
    return WaitDirectQueue();
}

bool DeferredRenderer::InitPrePostRender()
{
    CHECKED(
        RenderApi->GetDevice()
            ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&PrePostRenderData.CommandAllocator)),
        "Failed to create command allocator"
    )

    CHECKED(
        RenderApi->GetDevice()
            ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                PrePostRenderData.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&PrePostRenderData.CommandList)),
        "Failed to create command list"
    )

    CHECKED_S(PrePostRenderData.CommandList->Close());

    return true;
}

bool DeferredRenderer::PreRender(const Core::SceneView& SceneView)
{
    CHECKED_S(PrePostRenderData.CommandAllocator->Reset());

    CHECKED(
        PrePostRenderData.CommandList->Reset(PrePostRenderData.CommandAllocator.Get(), nullptr),
        "Can't reset command list"
    )

    SceneView.TransitionIntoRenderTargetState(PrePostRenderData.CommandList.Get());

    CHECKED(
        PrePostRenderData.CommandList->Close(),
        "Failed to close command list"
    )

    ID3D12CommandList* CommandLists[] = {PrePostRenderData.CommandList.Get()};

    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    // Initialize Point Lights for shadow mapping if enabled
    for (const auto& PointLight : Scene->GetPointLights())
    {
        if (!PointLight->HasShadowResources())
            PointLight->CreateShadowCubeMapResource(*RenderApi);
    }

    return true;
}

bool DeferredRenderer::PostRender(const Core::SceneView& SceneView)
{
    CHECKED(
        PrePostRenderData.CommandList->Reset(PrePostRenderData.CommandAllocator.Get(), nullptr),
        "Can't reset command list"
    )

    SceneView.TransitionIntoOriginalState(PrePostRenderData.CommandList.Get());

    CHECKED(
        PrePostRenderData.CommandList->Close(),
        "Failed to close command list"
    )

    ID3D12CommandList* CommandLists[] = {PrePostRenderData.CommandList.Get()};

    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    return true;
}

bool DeferredRenderer::WaitDirectQueue()
{
    CHECKED(
        RenderApi->GetDirectQueue()
            ->Signal(FrameFence.Get(), ++FrameFenceValue),
        "Failed to signal Fence"
    )

    CHECKED(
        FrameFence->SetEventOnCompletion(FrameFenceValue, nullptr),
        "Failed to wait for Fence"
    )

    return true;
}

const std::array<CD3DX12_STATIC_SAMPLER_DESC, 2>& DeferredRenderer::GetCommonStaticSamplers()
{
    bool bInitialized = false;
    static std::array<CD3DX12_STATIC_SAMPLER_DESC, 2> StaticSamplers{};

    if (!bInitialized)
    {
        StaticSamplers[0].Init(0); // Anisotropic
        StaticSamplers[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_POINT); // Point

        bInitialized = true;
    }

    return StaticSamplers;
}

bool DeferredRenderer::InitEmptyTexture()
{
    {
        D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        D3D12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1);

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommittedResource(
                    &HeapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &ResourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&EmptyTexture)
                ),
            "Can't create empty texture"
        )
    }

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> InitCommandAllocator {};
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> InitCommandList {};

    CHECKED(
        RenderApi->GetDevice()
        ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&InitCommandAllocator)),
        "Failed to create command allocator"
    )

    CHECKED(
        RenderApi->GetDevice()
        ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            InitCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&InitCommandList)),
        "Failed to create command list"
    )

    Microsoft::WRL::ComPtr<ID3D12Resource> UploadBuffer {};
    {
        D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(1);

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommittedResource(
                    &HeapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &ResourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&UploadBuffer)
                ),
            "Can't create upload buffer"
        )

    }

    // One zeroes RGBA pixel
    const std::byte Data[4] = {};

    D3D12_SUBRESOURCE_DATA SubResourceData {
        .pData = &Data,
        .RowPitch = 1,
        .SlicePitch = 1
    };
    UpdateSubresources(
        InitCommandList.Get(),
        EmptyTexture.Get(),
        UploadBuffer.Get(),
        0,
        0,
        1,
        &SubResourceData
    );

    ID3D12CommandList* CommandLists[] = {PrePostRenderData.CommandList.Get()};
    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        .NumDescriptors = 1,
        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    };

    CHECKED(
        RenderApi->GetDevice()
            ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&CpuEmptyTextureHeap)),
        "Failed to create descriptor heap for empty texture"
    )

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM);

    RenderApi->GetDevice()
        ->CreateShaderResourceView(EmptyTexture.Get(), &SRVDesc, CpuEmptyTextureHeap->GetCPUDescriptorHandleForHeapStart());

    return WaitDirectQueue();
}

bool DeferredRenderer::InitGBufferForView(const Core::SceneView& SceneView)
{
    const glm::ivec2 ViewportSize = SceneView.GetViewportSize();

    if (GBuffer.Size == ViewportSize)
        return true;

    // Allocate textures

    auto CreateTextureBuffer = [&](Microsoft::WRL::ComPtr<ID3D12Resource>& TextureBuffer, DXGI_FORMAT Format, const std::wstring_view& Name, bool bIsDepth = false) -> bool
    {
        D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        D3D12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(Format, ViewportSize.x, ViewportSize.y, 1, 1);

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.Format = Format;

        D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_RENDER_TARGET;

        if (bIsDepth)
        {
            ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            ClearValue.DepthStencil.Depth = 1.0f;
            ClearValue.DepthStencil.Stencil = 0;
            State = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        }
        else
        {
            ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            ClearValue.Color[0] = 0.f;
            ClearValue.Color[1] = 0.f;
            ClearValue.Color[2] = 0.f;
            ClearValue.Color[3] = 1.f;
        }

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommittedResource(
                    &HeapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &ResourceDesc,
                    State,
                    &ClearValue,
                    IID_PPV_ARGS(&TextureBuffer)
                ),
            "Can't create texture for gbuffer"
        )

        TextureBuffer->SetName(Name.data());

        return true;
    };

    const bool bError = !CreateTextureBuffer(GBuffer.DiffuseTexture, DXGI_FORMAT_R8G8B8A8_UNORM, L"GBuffer Diffuse")
        || !CreateTextureBuffer(GBuffer.WorldPositionTexture, DXGI_FORMAT_R32G32B32A32_FLOAT, L"GBuffer World Position")
        || !CreateTextureBuffer(GBuffer.WorldNormalTexture, DXGI_FORMAT_R32G32B32A32_FLOAT, L"GBuffer World Normal")
        || !CreateTextureBuffer(GBuffer.MetallicTexture, DXGI_FORMAT_R8G8B8A8_UNORM, L"GBuffer Metallic")
        || !CreateTextureBuffer(GBuffer.RoughnessTexture, DXGI_FORMAT_R8G8B8A8_UNORM, L"GBuffer Roughness")
        || !CreateTextureBuffer(GBuffer.EmissiveTexture, DXGI_FORMAT_R8G8B8A8_UNORM, L"GBuffer Emissive")
        || !CreateTextureBuffer(GBuffer.DepthStencilTexture, DXGI_FORMAT_D24_UNORM_S8_UINT, L"GBuffer DepthStencil", true);

    if (bError)
        return false;

    const std::array Textures {
        std::addressof(GBuffer.DiffuseTexture),
        std::addressof(GBuffer.WorldPositionTexture),
        std::addressof(GBuffer.WorldNormalTexture),
        std::addressof(GBuffer.MetallicTexture),
        std::addressof(GBuffer.RoughnessTexture),
        std::addressof(GBuffer.EmissiveTexture)
    };

    // Create RTV descriptors
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = 6,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&GBuffer.CpuRtvDescriptorHeap)),
            "Can't create cpu rtv descriptor heap for gbuffer"
        )

        CD3DX12_CPU_DESCRIPTOR_HANDLE Handle {GBuffer.CpuRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()};
        const unsigned IncrementSize = RenderApi->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (const auto & Texture: Textures)
        {
            RenderApi->GetDevice()
                ->CreateRenderTargetView(Texture->Get(), nullptr, Handle);

            Handle.Offset(1, IncrementSize);
        }
    }

    // Create SRV descriptors
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = GBuffer.TEXTURES_COUNT,
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&GBuffer.CpuSrvDescriptorHeap)),
            "Can't create cpu srv descriptor heap for gbuffer"
        )

        CD3DX12_CPU_DESCRIPTOR_HANDLE Handle {GBuffer.CpuSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()};
        const unsigned IncrementSize = RenderApi->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (const auto & Texture: Textures)
        {
            RenderApi->GetDevice()
                ->CreateShaderResourceView(Texture->Get(), nullptr, Handle);

            Handle.Offset(1, IncrementSize);
        }
    }

    // Create DSV descriptor
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };

        CHECKED(
            RenderApi->GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&GBuffer.CpuDsvDescriptorHeap)),
            "Can't create cpu dsv descriptor heap for gbuffer"
        )

        CD3DX12_CPU_DESCRIPTOR_HANDLE Handle {GBuffer.CpuDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()};
        RenderApi->GetDevice()->CreateDepthStencilView(GBuffer.DepthStencilTexture.Get(), nullptr, Handle);
    }

    // Create command list for barrier transitions
    {
        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&GBuffer.PresentToReadTransitionAllocator)),
            "Failed to create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&GBuffer.ReadToPresentTransitionAllocator)),
            "Failed to create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                    GBuffer.PresentToReadTransitionAllocator.Get(), nullptr, IID_PPV_ARGS(&GBuffer.TransitionCommandList)),
            "Failed to create command list"
        )

        CHECKED_S(GBuffer.TransitionCommandList->Close());
    }

    GBuffer.Size = ViewportSize;

    return true;
}

bool DeferredRenderer::TransitionGBufferFromRenderTargetToReadState()
{
    const std::array Barriers = {
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.DiffuseTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.WorldPositionTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.WorldNormalTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.MetallicTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.RoughnessTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.EmissiveTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ),
    };

    CHECKED_S(GBuffer.PresentToReadTransitionAllocator->Reset());
    CHECKED_S(GBuffer.TransitionCommandList->Reset(GBuffer.PresentToReadTransitionAllocator.Get(), nullptr))

    GBuffer.TransitionCommandList->ResourceBarrier(std::size(Barriers), Barriers.data());
    CHECKED_S(GBuffer.TransitionCommandList->Close());

    ID3D12CommandList* CommandLists[] = {GBuffer.TransitionCommandList.Get()};
    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    return true;
}

bool DeferredRenderer::TransitionGBufferFromReadToRenderTargetState()
{
    const std::array Barriers = {
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.DiffuseTexture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.WorldPositionTexture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.WorldNormalTexture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.MetallicTexture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.RoughnessTexture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(GBuffer.EmissiveTexture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET),
    };

    CHECKED_S(GBuffer.ReadToPresentTransitionAllocator->Reset());
    CHECKED_S(GBuffer.TransitionCommandList->Reset(GBuffer.ReadToPresentTransitionAllocator.Get(), nullptr))

    GBuffer.TransitionCommandList->ResourceBarrier(std::size(Barriers), Barriers.data());
    CHECKED_S(GBuffer.TransitionCommandList->Close());

    ID3D12CommandList* CommandLists[] = {GBuffer.TransitionCommandList.Get()};
    RenderApi->GetDirectQueue()
        ->ExecuteCommandLists(1, CommandLists);

    return true;
}

}
