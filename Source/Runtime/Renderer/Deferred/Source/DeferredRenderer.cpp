#include "Runtime/Renderer/Deferred/DeferredRenderer.h"
#include <array>
#include "Runtime/ModelLoader/Loader.h"
#include "Runtime/Renderer/Core/Scene/Scene.h"
#include "Runtime/Renderer/Core/Scene/SceneView.h"
#include "Runtime/Renderer/Core/TexturedMesh/Texture.h"
#include "nvtx3/nvtx3.hpp"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"

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

    if (!WaitDirectQueue())
        return false;

    return true;
}

bool DeferredRenderer::Render(const std::span<Core::SceneView>& SceneViews)
{
    for (const Core::SceneView& SceneView : SceneViews)
    {
        if (!PreRender(SceneView))
            return false;

        if (const auto [bSuccess, bRecordedCommands] = InitGBufferForView(SceneView); !bSuccess)
            return false;

        if (!GeometryPass(SceneView))
            return false;

        if (!AmbientDirectionalLightPass(SceneView))
            return false;

        if (!PointLightVolumesPass(SceneView))
            return false;

        if (!PostProcessingPass(SceneView))
            return false;

        if (!PostRender(SceneView))
            return false;

        WaitDirectQueue();
    }

    return true;
}

bool DeferredRenderer::GeometryPass(const Core::SceneView& SceneView)
{
    return true;
}

bool DeferredRenderer::AmbientDirectionalLightPass(const Core::SceneView& SceneView)
{
    return true;
}

bool DeferredRenderer::PointLightVolumesPass(const Core::SceneView& SceneView)
{
    return true;
}

bool DeferredRenderer::PostProcessingPass(const Core::SceneView& SceneView)
{
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
        *RenderApi.get(),
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
    WaitDirectQueue();

    return true;
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

DeferredRenderer::GBufferInitResult DeferredRenderer::InitGBufferForView(const Core::SceneView& SceneView)
{
    const glm::ivec2 ViewportSize = SceneView.GetViewportSize();

    if (GBuffer.Size == ViewportSize)
        return {true, false};

    // Allocate textures

    auto CreateTextureBuffer = [&](Microsoft::WRL::ComPtr<ID3D12Resource>& TextureBuffer, DXGI_FORMAT Format, bool bIsDepth = false) -> bool
    {
        D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        D3D12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(Format, ViewportSize.x, ViewportSize.y, 1, 1);

        if (bIsDepth)
            ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        else
            ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommittedResource(
                    &HeapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &ResourceDesc,
                    D3D12_RESOURCE_STATE_COMMON,
                    nullptr,
                    IID_PPV_ARGS(&TextureBuffer)
                ),
            "Can't create texture for gbuffer"
        )

        return true;
    };

    const bool bError = !CreateTextureBuffer(GBuffer.DiffuseTexture, DXGI_FORMAT_R8G8B8A8_UNORM)
        || !CreateTextureBuffer(GBuffer.WorldPositionTexture, DXGI_FORMAT_R32G32B32A32_FLOAT)
        || !CreateTextureBuffer(GBuffer.WorldNormalTexture, DXGI_FORMAT_R32G32B32A32_FLOAT)
        || !CreateTextureBuffer(GBuffer.MetallicTexture, DXGI_FORMAT_R8G8B8A8_UNORM)
        || !CreateTextureBuffer(GBuffer.RoughnessTexture, DXGI_FORMAT_R8G8B8A8_UNORM)
        || !CreateTextureBuffer(GBuffer.EmissiveTexture, DXGI_FORMAT_R8G8B8A8_UNORM)
        || !CreateTextureBuffer(GBuffer.DepthStencilTexture, DXGI_FORMAT_D24_UNORM_S8_UINT, true);

    if (bError)
        return {false, false};

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
            .NumDescriptors = 6,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
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

    GBuffer.Size = ViewportSize;

    return {true, true};
}

}
