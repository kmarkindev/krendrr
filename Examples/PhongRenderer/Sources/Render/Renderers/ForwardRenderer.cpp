#include "ForwardRenderer.h"
#include <d3dx12/d3dx12.h>
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
#include "Sources/Utils/Generators/TextureGenerator.h"

namespace kRendrr
{

    ForwardRenderer::ForwardRenderer(std::shared_ptr<kRendrr::RenderDevice> RenderDevice, std::shared_ptr<kRendrr::CommandQueue> CommandQueue)
        : RenderDevice(std::move(RenderDevice)), CommandQueue(std::move(CommandQueue)),
        CommandList(GetSharedPtrToStack(&CommandAllocator)), MeshPso(GetSharedPtrToStack(&MeshRootSignature))
    {

    }

    void ForwardRenderer::Initialize()
    {
        Fence.Initialize(*RenderDevice);
        CommandAllocator.Initialize(*RenderDevice);
        CommandList.Initialize(*RenderDevice);

        RenderSrvDescriptorHeap.Initialize(*RenderDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true);

        static constexpr auto CubeVertexArray = ConstexprDynamicContainerToArray<GenerateTriangleMeshVertices, true>();
        static constexpr auto CubeIndicesArray = ConstexprDynamicContainerToArray<GenerateTriangleMeshIndices>();
        static constexpr auto MeshTextureArray = ConstexprDynamicContainerToArray<GenerateCheckerTexture>();

        MeshVertexBuffer.Initialize(*RenderDevice, sizeof(CubeVertexArray));
        MeshVertexBuffer.GetBuffer()->SetName(L"Mesh Vertex Buffer") >> HResultCheck{};

        MeshIndexBuffer.Initialize(*RenderDevice, sizeof(CubeIndicesArray));
        MeshIndexBuffer.GetBuffer()->SetName(L"Mesh Index Buffer") >> HResultCheck{};

        MeshTexture.Initialize(*RenderDevice, DXGI_FORMAT_R8G8B8A8_UNORM, {64, 64}, 1);
        MeshTexture.GetTexture()->SetName(L"Mesh Texture") >> HResultCheck{};

        MeshUploadBuffer.Initialize(
            *RenderDevice,
            std::max(
                {
                    sizeof(CubeVertexArray),
                    sizeof(CubeIndicesArray),
                    GetRequiredIntermediateSize(MeshTexture.GetTexture().Get(), 0, 1)
                }
            )
        );
        MeshUploadBuffer.GetBuffer()->SetName(L"Mesh Upload Buffer") >> HResultCheck{};

        {
            // Load mesh data

            {
                MeshVertexBuffer.SetBufferSideAndStride(sizeof(CubeVertexArray), 5 * sizeof(float), std::size(CubeVertexArray));
                MeshUploadBuffer.UploadData(CubeVertexArray);
                MeshUploadBuffer.UploadDataToBuffer(*RenderDevice, *CommandQueue, MeshVertexBuffer, sizeof(CubeVertexArray));
            }

            {
                MeshIndexBuffer.SetBufferSizeAndFormat(sizeof(CubeIndicesArray), DXGI_FORMAT_R32_UINT, std::size(CubeIndicesArray));
                MeshUploadBuffer.UploadData(CubeIndicesArray);
                MeshUploadBuffer.UploadDataToBuffer(*RenderDevice, *CommandQueue, MeshIndexBuffer, sizeof(CubeIndicesArray));
            }
        }

        {
            // Compile shaders

            MeshVertexShader.InitializeFromFile("Shaders/ColoredMeshShader.hlsl", {
                .Target = "vs_5_1",
                .EntryPoint = "VSMain",
                .bCompileDebug = true
            });

            MeshPixelShader.InitializeFromFile("Shaders/ColoredMeshShader.hlsl", {
                .Target = "ps_5_1",
                .EntryPoint = "PSMain",
                .bCompileDebug = true
            });
        }

        {
            // Create PSO

            CD3DX12_ROOT_PARAMETER RootParams[1] = {};

            D3D12_DESCRIPTOR_RANGE DescriptorRange = {
                .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                .NumDescriptors = 1,
                .BaseShaderRegister = 0,
            };
            RootParams[0].InitAsDescriptorTable(1, &DescriptorRange);

            CD3DX12_STATIC_SAMPLER_DESC Samplers[1] = {
                {
                    0,
                    D3D12_FILTER_MIN_MAG_MIP_LINEAR
                }
            };

            MeshRootSignature.Initialize(
                *RenderDevice,
                RootParams,
                Samplers,
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
            );

            D3D12_INPUT_ELEMENT_DESC InputLayoutDescs[] = {
                {
                    "POS",
                    0,
                    DXGI_FORMAT_R32G32B32_FLOAT,
                    0,
                    D3D12_APPEND_ALIGNED_ELEMENT,
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    0
                },
                {
                    "UV",
                    0,
                    DXGI_FORMAT_R32G32_FLOAT,
                    0,
                    D3D12_APPEND_ALIGNED_ELEMENT,
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    0
                }
            };

            MeshPso.Initialize(*RenderDevice, {
                .VertexShader = MeshVertexShader,
                .PixelShader = MeshPixelShader,
                .InputLayout = {
                    .pInputElementDescs = InputLayoutDescs,
                    .NumElements = std::size(InputLayoutDescs)
                }
            });
        }

        {
            // Load Texture

            auto Desc = MeshTexture.GetSrvDesc();
            auto Handle = RenderSrvDescriptorHeap.GetCPUHandle(0);

            RenderDevice->GetDevice()
                ->CreateShaderResourceView(MeshTexture.GetTexture().Get(), &Desc, Handle);

            D3D12_SUBRESOURCE_DATA SubresourceData[] = {
                {
                    .pData = MeshTextureArray.data(),
                    .RowPitch = 64 * 4,
                    .SlicePitch = 64 * 4 * 64
                }
            };

            CommandList.GetList()
                ->Reset(CommandAllocator.GetAllocator().Get(), nullptr)
                >> HResultCheck {};

            if(UpdateSubresources(
                CommandList.GetList().Get(),
                MeshTexture.GetTexture().Get(),
                MeshUploadBuffer.GetBuffer().Get(),
                0,
                0,
                1,
                SubresourceData) == 0)
            {
                throw std::runtime_error("Failed to update texture subresource");
            }

            CommandList.GetList()
                ->Close()
                >> HResultCheck {};

            ID3D12CommandList* Lists[] = { CommandList.GetList().Get() };
            CommandQueue->GetQueue()
                ->ExecuteCommandLists(1, Lists);

            Fence.SignalQueue(*CommandQueue);
            Fence.WaitSignaledValueSpinlock();
        }

        {
            MeshUploadBuffer = {};

            CommandAllocator.GetAllocator()
                ->Reset()
                >> HResultCheck {};
        }

        {
            DepthStencilHeap.Initialize(*RenderDevice, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
        }
    }

    void ForwardRenderer::Render(const World& World, Viewport& Viewport)
    {
        if(!Viewport.GetSwapChain().HasRenderTarget())
        {
            return;
        }

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
            if(auto ViewportSize = Viewport.GetSize(); ViewportSize != DepthStencilSize)
            {
                InitializeDepthStencil(ViewportSize);
                DepthStencilSize = ViewportSize;
            }
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

            auto DepthStencilView = DepthStencilHeap.GetCPUHandle(0);

            CommandList.GetList()
                ->OMSetRenderTargets(std::size(RtvDescriptors), RtvDescriptors, true, &DepthStencilView);
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
            const glm::vec4 ClearColor = { 0.f, 0.f, 0.f, 1.f };

            CommandList.GetList()
                ->ClearRenderTargetView(RenderTargetView.GetCpuHandle(), &ClearColor.r, 0, nullptr);

            CommandList.GetList()
                ->ClearDepthStencilView(DepthStencilHeap.GetCPUHandle(0), D3D12_CLEAR_FLAG_DEPTH, 1.0, 0, 0, nullptr);
        }

        {
            CommandList.GetList()
                ->SetPipelineState(MeshPso.GetPso().Get());

            CommandList.GetList()
                ->SetGraphicsRootSignature(MeshRootSignature.GetRootSignature().Get());

            CommandList.GetList()
                ->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            auto VertexBufferView = MeshVertexBuffer.GetVertexBufferView();
            CommandList.GetList()
                ->IASetVertexBuffers(0, 1, &VertexBufferView);

            auto IndexBufferView = MeshIndexBuffer.GetIndexBufferView();
            CommandList.GetList()
                ->IASetIndexBuffer(&IndexBufferView);

            ID3D12DescriptorHeap* Heaps[] = { RenderSrvDescriptorHeap.GetDescriptorHeap().Get() };
            CommandList.GetList()
                ->SetDescriptorHeaps(1, Heaps);

            CommandList.GetList()
                ->SetGraphicsRootDescriptorTable(0, RenderSrvDescriptorHeap.GetGPUHandle(0));

            CommandList.GetList()
                ->DrawIndexedInstanced(MeshIndexBuffer.GetIndicesCount(), 1, 0, 0, 0);
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

    void ForwardRenderer::InitializeDepthStencil(glm::ivec2 Size)
    {
        constexpr static D3D12_CLEAR_VALUE ClearValue = {
            .Format = DXGI_FORMAT_D32_FLOAT,
            .DepthStencil = {
                .Depth = 1.0f,
                .Stencil = 0
            }
        };

        DepthStencilTexture = {};
        DepthStencilTexture.Initialize(*RenderDevice, DXGI_FORMAT_D32_FLOAT, Size, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &ClearValue);

        auto CpuHandle = DepthStencilHeap.GetCPUHandle(0);
        RenderDevice->GetDevice()
            ->CreateDepthStencilView(DepthStencilTexture.GetTexture().Get(), nullptr, CpuHandle);

        auto BarrierToWrite = CD3DX12_RESOURCE_BARRIER::Transition(
            DepthStencilTexture.GetTexture().Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_DEPTH_WRITE
        );

        CommandList.GetList()
            ->ResourceBarrier(1, &BarrierToWrite);
    }
}
