#include "Viewport.h"

Viewport::Viewport(std::shared_ptr<::RenderDevice> RenderDevice)
    : Viewport(std::move(RenderDevice), CreateDefaultWindow())
{
}

Viewport::Viewport(std::shared_ptr<::RenderDevice> RenderDevice, HWND Hwnd)
    : RenderDevice(std::move(RenderDevice)), Hwnd(Hwnd)
{

}

void Viewport::Initialize()
{
    // TODO: create swap chain and RTV using device
}

void Viewport::Uninitialize()
{

}

glm::vec2 Viewport::GetSize() const
{
    RECT Rect {};
    ::GetWindowRect(Hwnd, &Rect);

    return {Rect.right - 1, Rect.bottom - 1};
}

std::shared_ptr<RenderTargetView> Viewport::GetCurrentRenderTargetView() const
{
    return {};
}

void Viewport::PresentAndSwapCurrentRenderTargetView()
{

}

HWND Viewport::CreateDefaultWindow()
{
    WNDCLASSEX WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = DefWindowProc;
    WindowClass.hInstance = GetModuleHandle(nullptr);
    WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    WindowClass.lpszClassName = "DefaultWindow";
    ::RegisterClassEx(&WindowClass);

    RECT WindowRect = {0, 0, 1280, 720};
    ::AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

    return ::CreateWindow(
        WindowClass.lpszClassName,
        "Default Window",
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
}
