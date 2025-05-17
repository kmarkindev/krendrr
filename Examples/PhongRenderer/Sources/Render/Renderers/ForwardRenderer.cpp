#include "ForwardRenderer.h"
#include <d3dx12/d3dx12_barriers.h>
#include <d3dx12/d3dx12_core.h>
#include "glm/common.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/detail/func_trigonometric.inl"
#include "Sources/Platform/Viewport.h"
#include "Sources/Render/Resources/View/RenderTarget.h"
#include "Sources/Utils/Constexpr.h"
#include "Sources/Utils/HResultCheck.h"
#include "Sources/Utils/Memory.h"
#include "Sources/Utils/Generators/MeshGenerator.h"
#include "Sources/World/World.h"

namespace kRendrr
{

    ForwardRenderer::ForwardRenderer(std::shared_ptr<kRendrr::RenderDevice> RenderDevice, std::shared_ptr<kRendrr::CommandQueue> CommandQueue)
        : RenderDevice(std::move(RenderDevice)), CommandQueue(std::move(CommandQueue)),
        CommandList(GetSharedPtrToStack(&CommandAllocator))
    {

    }

    void ForwardRenderer::Render(const World& World, Viewport& Viewport)
    {
        const RenderTarget& RenderTargetView = Viewport
            .GetSwapChain()
            .GetCurrentRenderTargetView();

        {
            CommandAllocator.GetAllocator()
                ->Reset()
                >> HResultCheck {};

            CommandList.GetList()
                ->Reset(CommandAllocator.GetAllocator().Get(), nullptr)
                >> HResultCheck {};
        }

        {
            auto PresentToRtvBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                RenderTargetView.GetResource().Get(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET
            );
            CommandList.GetList()
                ->ResourceBarrier(1, &PresentToRtvBarrier);
        }

        {
            const D3D12_CPU_DESCRIPTOR_HANDLE RtvDescriptors[] = { RenderTargetView.GetCpuHandle() };

            CommandList.GetList()
                ->OMSetRenderTargets(std::size(RtvDescriptors), RtvDescriptors, true, nullptr);
        }

        {
            const D3D12_VIEWPORT D3dViewport = CD3DX12_VIEWPORT(0.f, 0.f, Viewport.GetSize().x, Viewport.GetSize().y);
            CommandList.GetList()
                ->RSSetViewports(1, &D3dViewport);

            D3D12_RECT ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);

            CommandList.GetList()
                ->RSSetScissorRects(1, &ScissorRect);
        }

        {
            const glm::vec4 ClearColor = { glm::sin(65), glm::sin(35), glm::sin(82), 1.f };

            CommandList.GetList()
                ->ClearRenderTargetView(RenderTargetView.GetCpuHandle(), &ClearColor.r, 0, nullptr);
        }

        {
            auto RtvToPresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                RenderTargetView.GetResource().Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT
            );
            CommandList.GetList()
                ->ResourceBarrier(1, &RtvToPresentBarrier);
        }

        {
            CommandList.GetList()
                ->Close()
                >> HResultCheck {};

            ID3D12CommandList* CommandLists[] = { CommandList.GetList().Get() };
            CommandQueue->GetQueue()->ExecuteCommandLists(std::size(CommandLists), CommandLists);
        }

        {
            Viewport.GetSwapChain().Present();

            Fence.SignalQueue(*CommandQueue);
            Fence.WaitSignaledValueSpinlock();
        }
    }

    void ForwardRenderer::Initialize()
    {
        Fence.Initialize(*RenderDevice);
        CommandAllocator.Initialize(*RenderDevice);
        CommandList.Initialize(*RenderDevice);

        constexpr auto CubeVertexArray = ConstexprDynamicContainerToArray<GenerateCubeMeshVertices, true>();
        constexpr auto CubeIndicesArray = ConstexprDynamicContainerToArray<GenerateCubeMeshIndices>();

        CubeVertexBuffer.Initialize(*RenderDevice, sizeof(CubeVertexArray));
        CubeVertexBuffer.GetBuffer()->SetName(L"Cube Vertex Buf");
        CubeIndexBuffer.Initialize(*RenderDevice, sizeof(CubeIndicesArray));
        CubeIndexBuffer.GetBuffer()->SetName(L"Cube Index Buf");
        CubeUploadBuffer.Initialize(*RenderDevice, std::max(sizeof(CubeVertexArray), sizeof(CubeIndicesArray)));
        CubeUploadBuffer.GetBuffer()->SetName(L"Cube Upload Buf");

        CubeVertexBuffer.SetBufferSideAndStride(sizeof(CubeVertexArray), 5 * sizeof(float), std::size(CubeVertexArray));
        CubeIndexBuffer.SetBufferSizeAndFormat(sizeof(CubeIndexBuffer), DXGI_FORMAT_R32_UINT, std::size(CubeIndicesArray));

        {
            CubeUploadBuffer.UploadData(CubeVertexArray);

            CommandList.GetList()
                ->Reset(CommandAllocator.GetAllocator().Get(), nullptr)
                >> HResultCheck {};

            CommandList.GetList()
                ->CopyBufferRegion(
                    CubeVertexBuffer.GetBuffer().Get(),
                    0,
                    CubeUploadBuffer.GetBuffer().Get(),
                    0,
                    sizeof(CubeVertexArray)
                );

            CommandList.GetList()
                ->Close()
                >> HResultCheck {};

            ID3D12CommandList* List[] = {CommandList.GetList().Get()};
            CommandQueue->GetQueue()
                ->ExecuteCommandLists(1, List);

            Fence.SignalQueue(*CommandQueue);
            Fence.WaitSignaledValueSpinlock();
        }

        {
            CubeUploadBuffer.UploadData(CubeIndicesArray);

            CommandList.GetList()
                ->Reset(CommandAllocator.GetAllocator().Get(), nullptr)
                >> HResultCheck {};

            CommandList.GetList()
                ->CopyBufferRegion(
                    CubeIndexBuffer.GetBuffer().Get(),
                    0,
                    CubeUploadBuffer.GetBuffer().Get(),
                    0,
                    sizeof(CubeIndicesArray)
                );

            CommandList.GetList()
                ->Close()
                >> HResultCheck {};

            ID3D12CommandList* List[] = { CommandList.GetList().Get() };
            CommandQueue->GetQueue()
                ->ExecuteCommandLists(1, List);

            Fence.SignalQueue(*CommandQueue);
            Fence.WaitSignaledValueSpinlock();
        }

        {
            CubeUploadBuffer = {};

            CommandAllocator.GetAllocator()
                ->Reset()
                >> HResultCheck {};
        }
    }

}
