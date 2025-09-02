#include "Runtime/Renderer/Deferred/DeferredRenderer.h"
#include <array>
#include "Runtime/ModelLoader/Loader.h"
#include "Runtime/Renderer/Core/Lights/PointLight.h"
#include "Runtime/Renderer/Core/Scene/Scene.h"
#include "Runtime/Renderer/Core/Scene/SceneView.h"
#include "Runtime/Renderer/Core/TexturedMesh/Texture.h"
#include "nvtx3/nvtx3.hpp"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"

namespace krendrr::Runtime::Renderer::Deferred
{

bool DeferredRenderer::Initialize(std::shared_ptr<RenderApi::Core::RenderApi> NewRenderApi, std::shared_ptr<Core::Scene> NewScene)
{
    RenderApi = std::move(NewRenderApi);
    Scene = std::move(NewScene);

    nvtx3::scoped_range InitRange {"Deferred Renderer: Initialize"};

    CHECKED(
        RenderApi->GetDevice()
            ->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&RenderFence)),
        "Failed to create Fence"
    )

    return true;
}

bool DeferredRenderer::Render(const std::span<Core::SceneView>& SceneViews)
{
    for (const Core::SceneView& SceneView : SceneViews)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetHandle = SceneView.GetRenderTargetHandle();

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator {};
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList {};

        CHECKED(
        RenderApi->GetDevice()
            ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)),
            "Failed to create command allocator"
        )

        CHECKED(
            RenderApi->GetDevice()
                ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList)),
            "Failed to create command list"
        )

        {
            const GBufferInitResult Result = InitGBufferForView(SceneView, CommandList.Get());

            if (!Result.bSuccess)
            {
                // TODO: log error
                return false;
            }
        }

        SceneView.TransitionIntoRenderTargetState(CommandList.Get());

        const static D3D12_RECT ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
        D3D12_VIEWPORT SceneViewViewport = SceneView.GetD3dViewport();

        CommandList->RSSetViewports(1, &SceneViewViewport);

        constexpr FLOAT ClearColor[4] = {0.3, 0.5, 0.25, 1.0};
        CommandList->ClearRenderTargetView(RenderTargetHandle, ClearColor, 0, nullptr);

        CommandList->OMSetRenderTargets(1, &RenderTargetHandle, true, nullptr);

        SceneView.TransitionIntoOriginalState(CommandList.Get());

        CHECKED_S(CommandList->Close());
        ID3D12CommandList* CommandLists[] = {CommandList.Get()};

        RenderApi->GetDirectQueue()
            ->ExecuteCommandLists(std::size(CommandLists), CommandLists);

        WaitDirectQueue();
    }

    return true;
}

bool DeferredRenderer::Shutdown()
{
    return WaitDirectQueue();
}

bool DeferredRenderer::WaitDirectQueue()
{
    CHECKED(
        RenderApi->GetDirectQueue()
            ->Signal(RenderFence.Get(), ++RenderFenceValue),
        "Failed to signal Fence"
    )

    CHECKED(
        RenderFence->SetEventOnCompletion(RenderFenceValue, nullptr),
        "Failed to wait for Fence"
    )

    return true;
}

DeferredRenderer::GBufferInitResult DeferredRenderer::InitGBufferForView(const Core::SceneView& SceneView, ID3D12GraphicsCommandList* CommandList)
{
    const glm::ivec2 ViewportSize = SceneView.GetViewportSize();

    if (GBuffer.Size == ViewportSize)
        return {true, false};

    auto CreateTextureBuffer = [&](Microsoft::WRL::ComPtr<ID3D12Resource>& TextureBuffer, DXGI_FORMAT Format) -> bool
    {
        D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        D3D12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(Format, ViewportSize.x, ViewportSize.y, 1, 1);

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
        || !CreateTextureBuffer(GBuffer.WorldPositionTexture, DXGI_FORMAT_R32G32B32_FLOAT)
        || !CreateTextureBuffer(GBuffer.WorldNormalTexture, DXGI_FORMAT_R32G32B32_FLOAT)
        || !CreateTextureBuffer(GBuffer.MetallicTexture, DXGI_FORMAT_R8_UNORM)
        || !CreateTextureBuffer(GBuffer.RoughnessTexture, DXGI_FORMAT_R8_UNORM)
        || !CreateTextureBuffer(GBuffer.EmissiveTexture, DXGI_FORMAT_R8G8B8A8_UNORM)
        || !CreateTextureBuffer(GBuffer.DepthStencilTexture, DXGI_FORMAT_D24_UNORM_S8_UINT);

    if (bError)
        return {false, false};

    std::array Textures {
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

        CD3DX12_CPU_DESCRIPTOR_HANDLE Handle {GBuffer.CpuSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()};
        RenderApi->GetDevice()->CreateDepthStencilView(GBuffer.DepthStencilTexture.Get(), nullptr, Handle);
    }

    GBuffer.Size = ViewportSize;

    return {true, true};
}

}
