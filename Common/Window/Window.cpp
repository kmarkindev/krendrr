#include "Window.h"

#include <string_view>

Window::Window()
    : Window({800, 600}, "Main Window")
{
}

Window::Window(SIZE WindowSize, std::string_view WindowTitle)
{
    InitializeWindow(WindowSize, WindowTitle);
}

void Window::ShowWindow()
{
    ::ShowWindow(WindowHandle, SW_SHOWNORMAL);
}

HWND Window::GetWindowHandle() const
{
    return WindowHandle;
}
RECT Window::GetWindowRect() const
{
    RECT WindowRect {};
    ::GetClientRect(WindowHandle, &WindowRect);

    return WindowRect;
}

void Window::InitializeWindow(SIZE WindowSize, std::string_view WindowTitle)
{
    WNDCLASSEX WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = GetModuleHandle(nullptr);
    WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    WindowClass.lpszClassName = "MainWindow";
    ::RegisterClassEx(&WindowClass);

    RECT WindowRect = {0, 0, WindowSize.cx, WindowSize.cy};
    ::AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

    WindowHandle = ::CreateWindow(
        WindowClass.lpszClassName,
        WindowTitle.data(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WindowRect.right - WindowRect.left,
        WindowRect.bottom - WindowRect.top,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr);
}

LRESULT Window::WindowProc(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
    }

    return ::DefWindowProc(Hwnd, Message, WParam, LParam);
}
