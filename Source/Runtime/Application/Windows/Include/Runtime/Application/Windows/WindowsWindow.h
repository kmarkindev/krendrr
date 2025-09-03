#pragma once
#include <array>
#include <dxgi1_6.h>
#include "Runtime/Application/Core/Window.h"
#include "SDL3/SDL_video.h"

namespace krendrr::Runtime::Application::Windows
{
    class WindowsWindow final : public Core::Window
    {
    public:

        constexpr static int SWAP_CHAIN_BUFFER_COUNT = 2;
        constexpr static const char* SLD_WINDOW_OBJECT_PROPERTY = "window_object";

        bool Initialize(const std::shared_ptr<RenderApi::Core::RenderApi>& NewRenderApi, const InitializeParams& Params) override;

        bool Destroy() override;

        [[nodiscard]] bool IsValid() const override;

        ~WindowsWindow() override;

        [[nodiscard]] glm::ivec2 GetSize() const override;

        bool Swap() override;

        void HandleWindowSizeChanged() override;

        WindowRenderData GetCurrentRenderTargetView() const override;

    private:

        std::shared_ptr<RenderApi::Core::RenderApi> RenderApi {};

        int DescriptorIncrementSize {-1};

        SDL_Window* Window {};
        HWND WindowHandle {};

        Microsoft::WRL::ComPtr<IDXGISwapChain4> SwapChain {};
        int SwapChainBufferIndex {};

        std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, SWAP_CHAIN_BUFFER_COUNT> RenderTargets {};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RtvCpuDescriptorHeap {};

        bool CreateUpdateSwapChain();
        bool CreateUpdateRenderTargets();
    };
}

