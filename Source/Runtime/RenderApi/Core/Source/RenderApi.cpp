#include "Runtime/RenderApi/Core/RenderApi.h"

#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include "Runtime/RenderApi/Core/ApiCallCheck.h"

namespace krendrr::Runtime::RenderApi::Core
{
    bool RenderApi::Initialize(const InitParams& Params)
    {
        if (IsValid())
        {
            // TODO: log error
            return false;
        }

        UINT DxgiFactoryFlags = 0;

        if (!SetupDebugLayer(DxgiFactoryFlags, Params))
            return false;

        if (!CreateDevice(DxgiFactoryFlags))
            return false;

        if (!CreateCommandQueues())
            return false;

        bShadersDebugEnabled = Params.bEnableShadersDebug;

        return true;
    }

    bool RenderApi::IsValid() const
    {
        return D3dDevice != nullptr && D3dDirectCommandQueue != nullptr;
    }

    bool RenderApi::Shutdown()
    {
        if (!IsValid())
        {
            // TODO: log error
            return false;
        }

        return true;
    }

    Microsoft::WRL::ComPtr<ID3D12Device> RenderApi::GetDevice() const
    {
        return D3dDevice;
    }

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> RenderApi::GetDirectQueue() const
    {
        return D3dDirectCommandQueue;
    }

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> RenderApi::GetCopyQueue() const
    {
        return D3dCopyCommandQueue;
    }

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> RenderApi::GetComputeQueue() const
    {
        return D3dComputeCommandQueue;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory6> RenderApi::GetDXGIFactory() const
    {
        return DxgiFactory;
    }

    bool RenderApi::IsShadersDebugEnabled() const
    {
        return bShadersDebugEnabled;
    }

    unsigned RenderApi::GetShaderCompileFlags() const
    {
#if defined(_DEBUG)
        if (IsShadersDebugEnabled())
        {
            return D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        }
#endif

        return 0;
    }

    bool RenderApi::WaitForQueue(ID3D12CommandQueue* Queue) const
    {
        Microsoft::WRL::ComPtr<ID3D12Fence> Fence {};

        CHECKED_S(
            GetDevice()
                ->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence))
        )

        CHECKED_S(
            Queue->Signal(Fence.Get(), 1)
        )

        CHECKED_S(
            Fence->SetEventOnCompletion(1, nullptr)
        )

        return true;
    }

    const RenderApi::BufferLayout& RenderApi::GetCommonMeshBufferLayout() const
    {
        static BufferLayout Layout = {
            .Stride = 11 * sizeof(float),
            .Layout = {
                D3D12_INPUT_ELEMENT_DESC {
                    "POSITION",
                    0,
                    DXGI_FORMAT_R32G32B32_FLOAT,
                    0,
                    D3D12_APPEND_ALIGNED_ELEMENT,
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    0
                },
                D3D12_INPUT_ELEMENT_DESC {
                    "UV",
                    0,
                    DXGI_FORMAT_R32G32_FLOAT,
                    0,
                    D3D12_APPEND_ALIGNED_ELEMENT,
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    0
                },
                D3D12_INPUT_ELEMENT_DESC {
                    "NORMAL",
                    0,
                    DXGI_FORMAT_R32G32B32_FLOAT,
                    0,
                    D3D12_APPEND_ALIGNED_ELEMENT,
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    0
                },
                D3D12_INPUT_ELEMENT_DESC {
                    "TANGENT",
                    0,
                    DXGI_FORMAT_R32G32B32_FLOAT,
                    0,
                    D3D12_APPEND_ALIGNED_ELEMENT,
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    0
                }
            }
        };

        return Layout;
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> RenderApi::CreateUploadBufferAndMap(const std::span<const std::byte>& Data, bool bSkipMap) const
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> UploadBuffer {};

        const CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(Data.size_bytes());

        CHECKED(
            GetDevice()
                ->CreateCommittedResource(
                    &HeapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &ResourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&UploadBuffer)
                ),
            "Failed to create upload buffer"
        )

        if (!bSkipMap)
        {
            void* MappedPtr {};
            CHECKED_S(UploadBuffer->Map(0, nullptr, &MappedPtr));
            std::memcpy(MappedPtr, Data.data(), Data.size_bytes());
            UploadBuffer->Unmap(0, nullptr);
        }

        return UploadBuffer;
    }

    bool RenderApi::SetupDebugLayer(UINT& DxgiFactoryFlags, const InitParams& Params)
    {
#ifdef _DEBUG
        if (Params.Debug != InitParams::Debug::None)
        {
            Microsoft::WRL::ComPtr<ID3D12Debug1> DebugController {};

            CHECKED(
                D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)),
                "Can't create debug interface"
            )

            DebugController->EnableDebugLayer();

            if (Params.Debug == InitParams::Debug::DebugLayerWithGpuBasedValidation)
                DebugController->SetEnableGPUBasedValidation(true);

            DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#endif

        return true;
    }

    bool RenderApi::CreateDevice(UINT DxgiFactoryFlags)
    {
        CHECKED(
            CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(&DxgiFactory)),
            "Failed to create DXGI factory"
        )

        HRESULT DeviceCreationResult = D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_12_1,
            IID_PPV_ARGS(&D3dDevice)
        );

        if (DeviceCreationResult == DXGI_ERROR_UNSUPPORTED)
        {
            // TODO: log error DirectX 12 is not supported on current machine
            return false;
        }

        CHECKED(
            DeviceCreationResult,
            "Failed to create D3D device"
        )

        return true;
    }

    bool RenderApi::CreateCommandQueues()
    {
        // Direct
        {
            D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
            QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

            CHECKED(
                D3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&D3dDirectCommandQueue)),
                "Failed to create direct command queue"
            )

            D3dDirectCommandQueue->SetName(L"Direct Command Queue");
        }

        // Copy
        {
            D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
            QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

            CHECKED(
                D3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&D3dCopyCommandQueue)),
                "Failed to create copy command queue"
            )

            D3dCopyCommandQueue->SetName(L"Copy Command Queue");
        }

        // Compute
        {
            D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
            QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

            CHECKED(
                D3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&D3dComputeCommandQueue)),
                "Failed to create Compute command queue"
            )

            D3dComputeCommandQueue->SetName(L"Compute Command Queue");
        }

        return true;
    }
}
