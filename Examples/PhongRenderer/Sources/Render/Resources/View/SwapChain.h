#pragma once

#include <cstdint>
#include <d3d12.h>
#include <dxgi1_5.h>
#include <vector>
#include <Windows.h>
#include <wrl/client.h>
#include "Sources/Render/Resources/RenderResource.h"
#include "RenderTarget.h"

namespace kRendrr
{
    class CommandQueue;

    class SwapChain : public RenderResource
    {
    public:

        explicit SwapChain(HWND Hwnd);

        void Initialize(const RenderDevice& RenderDevice, const CommandQueue& CommandQueue);

        [[nodiscard]] const RenderTarget& GetCurrentRenderTargetView() const;

        void Present();

    private:

        HWND Hwnd {};
        uint8_t BuffersCount {};

        Microsoft::WRL::ComPtr<IDXGISwapChain4> DxgiSwapChain {};
        
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuRenderTargetsHeap {};
        /**
         * Constructed from resources, provided by Swap Chain and stored in one CPU Descriptor Heap (CpuRenderTargetsHeap),
         * that should outlive them.
         */
        std::vector<RenderTarget> RenderTargets {};

    };
}
