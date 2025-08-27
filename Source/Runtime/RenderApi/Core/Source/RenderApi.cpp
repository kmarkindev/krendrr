#include "Runtime/RenderApi/Core/RenderApi.h"
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

        if (!CreateCommandQueue())
            return false;

        return true;
    }

    bool RenderApi::IsValid() const
    {
        return D3dDevice != nullptr && D3dCommandQueue != nullptr;
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

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> RenderApi::GetCommandQueue() const
    {
        return D3dCommandQueue;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory6> RenderApi::GetDXGIFactory() const
    {
        return DxgiFactory;
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
        Microsoft::WRL::ComPtr<IDXGIFactory6> Factory {};

        CHECKED(
            CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(&Factory)),
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

    bool RenderApi::CreateCommandQueue()
    {
        D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
        QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        CHECKED(
            D3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&D3dCommandQueue)),
            "Failed to create command queue"
        )

        return true;
    }
}
