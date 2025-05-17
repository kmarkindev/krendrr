#include "Viewport.h"

#include "Sources/Render/RenderDevice.h"
#include "Sources/Render/Resources/Commands/CommandQueue.h"
#include "Sources/Render/Resources/View/SwapChain.h"

namespace kRendrr
{

    Viewport::Viewport(std::shared_ptr<RenderDevice> RenderDevice)
    : Viewport(std::move(RenderDevice), "Window", {1200, 720})
    {
    }

    Viewport::Viewport(std::shared_ptr<RenderDevice> RenderDevice, std::string_view WindowName, glm::ivec2 WindowSize, glm::ivec2 WindowPos)
        : Viewport(std::move(RenderDevice), CreateDefaultWindow(WindowName, WindowSize, WindowPos))
    {
    }

    Viewport::Viewport(std::shared_ptr<RenderDevice> RenderDevice, HWND Hwnd)
        : Hwnd(Hwnd), SwapChain(std::move(RenderDevice), Hwnd)
    {

    }

    void Viewport::Initialize(const CommandQueue& CommandQueue)
    {
        SwapChain.Initialize(CommandQueue);

        ::SetWindowLongPtr(Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        ::ShowWindow(Hwnd, SW_SHOWNORMAL);
    }

    glm::vec2 Viewport::GetSize() const
    {
        RECT Rect {};
        ::GetClientRect(Hwnd, &Rect);

        return {Rect.right - Rect.left, Rect.bottom - Rect.top};
    }

    const SwapChain& Viewport::GetSwapChain() const
    {
        return SwapChain;
    }

    SwapChain& Viewport::GetSwapChain()
    {
        return SwapChain;
    }

    void Viewport::HandleHwndResize()
    {
        if(!bCatchedResizeEventRecently)
        {
            return;
        }

        bCatchedResizeEventRecently = false;
        SwapChain.OnHwndChangedSize();
    }

    void Viewport::CatchedResizeEvent()
    {
        bCatchedResizeEventRecently = true;
    }

    LRESULT CALLBACK Viewport::DefaultWindowWndProc(
        HWND Hwnd,
        UINT Msg,
        WPARAM WParam,
        LPARAM LParam
    )
    {
        auto* Self = reinterpret_cast<Viewport*>(GetWindowLongPtr(Hwnd, GWLP_USERDATA));

        switch (Msg) {

            case WM_SIZE:
                Self->CatchedResizeEvent();
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_SHOWWINDOW:
                if(Self && Self->GetSwapChain().IsInitialized())
                {
                    if(WParam == TRUE)
                    {
                        Self->GetSwapChain().OnHwndShown();
                    }
                    else if (WParam == FALSE)
                    {
                        Self->GetSwapChain().OnHwndHidden();
                    }
                }
                return 0;
        }

        return DefWindowProc(Hwnd, Msg, WParam, LParam);
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

        return Handle;
    }

}
