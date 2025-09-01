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

        SceneView.TransitionIntoRenderTargetState(CommandList.Get());

        const static D3D12_RECT ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
        D3D12_VIEWPORT SceneViewViewport = SceneView.GetD3dViewport();

        CommandList->RSSetViewports(1, &SceneViewViewport);
        CommandList->RSSetScissorRects(1, &ScissorRect);

        FLOAT ClearColor[4] = {0.3, 0.5, 0.25, 1.0};
        CommandList->ClearRenderTargetView(RenderTargetHandle, ClearColor, 0, nullptr);

        CommandList->OMSetRenderTargets(1, &RenderTargetHandle, true, nullptr);

        SceneView.TransitionIntoOriginalState(CommandList.Get());

        CommandList->Close();
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
            ->Signal(RenderFence.Get(), ++FenceValue),
        "Failed to signal Fence"
    )

    CHECKED(
        RenderFence->SetEventOnCompletion(FenceValue, nullptr),
        "Failed to wait for Fence"
    )
}
}
