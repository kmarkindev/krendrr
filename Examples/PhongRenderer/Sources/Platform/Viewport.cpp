#include "Viewport.h"
#include "Sources/Render/RenderDevice.h"
#include "Sources/Render/Resources/Commands/CommandQueue.h"
#include "Sources/Render/Resources/View/SwapChain.h"

namespace kRendrr
{

    Viewport::Viewport()
    : Viewport("Window", {1200, 720})
    {
    }

    Viewport::Viewport(std::string_view WindowName, glm::ivec2 WindowSize, glm::ivec2 WindowPos)
        : Viewport(CreateDefaultWindow(WindowName, WindowSize, WindowPos))
    {
    }

    Viewport::Viewport(HWND Hwnd)
        : Hwnd(Hwnd), SwapChain(Hwnd)
    {

    }

    void Viewport::Initialize(const RenderDevice& RenderDevice, const CommandQueue& CommandQueue)
    {
        SwapChain.Initialize(RenderDevice, CommandQueue);
    }

    glm::vec2 Viewport::GetSize() const
    {
        RECT Rect {};
        ::GetWindowRect(Hwnd, &Rect);

        return {Rect.right - 1, Rect.bottom - 1};
    }

    const SwapChain& Viewport::GetSwapChain() const
    {
        return SwapChain;
    }

    SwapChain& Viewport::GetSwapChain()
    {
        return SwapChain;
    }

    LRESULT CALLBACK DefaultWindowWndProc(
        HWND Hwnd,
        UINT Msg,
        WPARAM WParam,
        LPARAM LParam
    )
    {
        switch (Msg) {

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            default:
                return DefWindowProc(Hwnd, Msg, WParam, LParam);
        }

        return 0;
    }

    HWND Viewport::CreateDefaultWindow(std::string_view WindowName, const glm::ivec2& WindowSize, glm::ivec2 WindowPos)
    {
        WNDCLASSEX WindowClass = {};
        WindowClass.cbSize = sizeof(WNDCLASSEX);
        WindowClass.style = CS_HREDRAW | CS_VREDRAW;
        WindowClass.lpfnWndProc = DefaultWindowWndProc;
        WindowClass.hInstance = GetModuleHandle(nullptr);
        WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        WindowClass.lpszClassName = "DefaultViewportWindow";
        ::RegisterClassEx(&WindowClass);

        RECT WindowRect = {WindowPos.x, WindowPos.y, WindowSize.x, WindowSize.y};
        ::AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

        HWND Handle = ::CreateWindow(
            WindowClass.lpszClassName,
            WindowName.data(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            WindowRect.right - WindowRect.left,
            WindowRect.bottom - WindowRect.top,
            nullptr,
            nullptr,
            GetModuleHandle(nullptr),
            nullptr
        );

        ::ShowWindow(Handle, SW_SHOWNORMAL);

        return Handle;
    }

}
