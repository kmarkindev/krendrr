#pragma once

#include <string_view>
#include <glm/vec2.hpp>
#include <Windows.h>
#include "Sources/Render/Resources/View/SwapChain.h"

namespace kRendrr
{
    class CommandQueue;
    class SwapChain;
    class RenderTarget;
    class RenderDevice;

    class Viewport
    {
    public:

        explicit Viewport(std::shared_ptr<RenderDevice> RenderDevice);

        Viewport(std::shared_ptr<RenderDevice> RenderDevice, std::string_view WindowName, glm::ivec2 WindowSize, glm::ivec2 WindowPos = {});

        explicit Viewport(std::shared_ptr<RenderDevice> RenderDevice, HWND Hwnd);

        void Initialize(const CommandQueue& CommandQueue);

        /**
         * Get viewport pixel size used for drawing (exluding borders, shadows, etc.)
         */
        [[nodiscard]] glm::ivec2 GetSize() const;

        [[nodiscard]] const SwapChain& GetSwapChain() const;

        [[nodiscard]] SwapChain& GetSwapChain();

        /**
         * Responsible for detecting HWND resize and making sure Swap Chain was updated
         */
        void HandleHwndResize();

    protected:

        void CatchedResizeEvent();

    private:

        HWND Hwnd {};

        SwapChain SwapChain;

        bool bCatchedResizeEventRecently { false };

        HWND CreateDefaultWindow(std::string_view WindowName, const glm::ivec2& WindowSize, glm::ivec2 WindowPos);

        static LRESULT CALLBACK DefaultWindowWndProc(
            HWND Hwnd,
            UINT Msg,
            WPARAM WParam,
            LPARAM LParam
        );

    };
}
