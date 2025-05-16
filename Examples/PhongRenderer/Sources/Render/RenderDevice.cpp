#include "RenderDevice.h"

#include <d3d12sdklayers.h>
#include <dxgi1_3.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <wrl/client.h>
#include "Sources/Utils/HResultCheck.h"

namespace kRendrr
{
    void RenderDevice::Initialize(const RenderDeviceInitParams& InitParams)
    {
        CheckInitialization(false);

        DxgiFlags = 0;

        if(InitParams.DebugMode != RenderDeviceInitParams::DebugMode::Disabled)
        {
            Microsoft::WRL::ComPtr<ID3D12Debug1> DebugController;

            D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))
                >> HResultCheck {};

            DebugController->EnableDebugLayer();

            if(InitParams.DebugMode == RenderDeviceInitParams::DebugMode::EnabledWithGpuBasedValidation)
            {
                DebugController->SetEnableGPUBasedValidation(true);
            }

            // Enable additional debug layers.
            DxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }

        Microsoft::WRL::ComPtr<IDXGIFactory6> Factory {};

        CreateDXGIFactory2(DxgiFlags, IID_PPV_ARGS(&Factory))
            >> HResultCheck{};

        D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_12_1,
            IID_PPV_ARGS(&Device)
        ) >> HResultCheck{};

        MarkAsInitialized();
    }

    uint8_t RenderDevice::GetDxgiFlags() const
    {
        CheckInitialization();

        return DxgiFlags;
    }

    Microsoft::WRL::ComPtr<ID3D12Device> RenderDevice::GetDevice() const
    {
        CheckInitialization();

        return Device;
    }
}