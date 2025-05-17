#pragma once

#include <cstdint>
#include <d3d12.h>
#include <dxgi1_5.h>
#include <memory>
#include <vector>
#include <Windows.h>
#include <wrl/client.h>
#include "Sources/Render/Resources/RenderResource.h"
#include "RenderTarget.h"

namespace kRendrr
{
    class CommandQueue;

    /**
     * Note: Render Target objects are created when window is first shown, and recreated when it has been resized.
     */
    class SwapChain : public RenderResource
    {
    public:

        explicit SwapChain(std::shared_ptr<RenderDevice> RenderDevice, HWND Hwnd);

        void Initialize(const CommandQueue& CommandQueue);

        /**
         * If window is not visible, then there is no render target.
         */
        bool HasRenderTarget() const;

        [[nodiscard]] const RenderTarget& GetCurrentRenderTargetView() const;

        void Present();

        void OnHwndChangedSize();

        void OnHwndShown();

        void OnHwndHidden();

    private:

        std::shared_ptr<RenderDevice> RenderDevice {};

        HWND Hwnd {};
        uint8_t BuffersCount {};

        Microsoft::WRL::ComPtr<IDXGISwapChain4> DxgiSwapChain {};
        
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuRenderTargetsHeap {};
        /**
         * Constructed from resources, provided by Swap Chain and stored in one CPU Descriptor Heap (CpuRenderTargetsHeap),
         * that should outlive them.
         */
        std::vector<RenderTarget> RenderTargets {};

        void ReleaseRenderTargets();

        void ObtainRenderTargets();

    };
}
