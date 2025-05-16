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

        Viewport();

        Viewport(std::string_view WindowName, glm::ivec2 WindowSize, glm::ivec2 WindowPos = {});

        explicit Viewport(HWND Hwnd);

        void Initialize(const RenderDevice& RenderDevice, const CommandQueue& CommandQueue);

        /**
         * Get viewport pixel size used for drawing (exluding borders, shadows, etc.)
         */
        [[nodiscard]] glm::vec2 GetSize() const;

        [[nodiscard]] const SwapChain& GetSwapChain() const;

        [[nodiscard]] SwapChain& GetSwapChain();

    private:

        HWND Hwnd {};

        SwapChain SwapChain;

        static HWND CreateDefaultWindow(std::string_view WindowName, const glm::ivec2& WindowSize, glm::ivec2 WindowPos);

    };
}
