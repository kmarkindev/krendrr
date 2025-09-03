#pragma once

#include <memory>
#include <string_view>

#include "Runtime/RenderApi/Core/RenderApi.h"
#include "glm/vec2.hpp"

namespace krendrr::Runtime::Application::Core
{
    class Application;

    /**
     * Represents window in current platform.
     *
     * It is NOT required to close this window before Application shutdown call or window's destructor call.
     * So feel free to save it into smart pointer after Window::Create call and forget about it.
     *
     * Note that some platforms may only have one window, in this case window allocation will fail.
     *
     * If platform always has a window (like browsers do), you are going to get access to it by creating window for the first time.
     *
     * You can call initialize again, after Close was called
     */
    class Window
    {
    public:

        struct InitializeParams
        {
            std::string_view Title {};
            glm::ivec2 Size {};
        };

        virtual ~Window() = default;

        static Window* Create(const std::shared_ptr<RenderApi::Core::RenderApi>& RenderApi, const InitializeParams& Params);

        virtual bool Initialize(const std::shared_ptr<RenderApi::Core::RenderApi>& RenderApi, const InitializeParams& Params) = 0;

        virtual bool Destroy() = 0;

        /**
         * Checks if this object holds valid window.
         * It returns false after window was Closed or if this object was not initialized.
         */
        [[nodiscard]] virtual bool IsValid() const = 0;

        [[nodiscard]] virtual glm::ivec2 GetSize() const = 0;

        virtual bool Swap() = 0;

        virtual void HandleWindowSizeChanged() = 0;

        struct WindowRenderData
        {
            Microsoft::WRL::ComPtr<ID3D12Resource> WindowRenderTarget {};
            D3D12_CPU_DESCRIPTOR_HANDLE Handle {};
        };

        /**
         * Returns Render Target View in D3D12_RESOURCE_STATE_PRESENT
         * Note: After Swap, buffer may change, so do not forget to update your SceneView objects
         *
         * Also, it is required to release render target pointer after swap. While it is alive, it is impossible to change window size correctly.
         */
        virtual WindowRenderData GetCurrentRenderTargetView() const = 0;
    };
}

