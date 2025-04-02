#pragma once

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <string_view>

class Window 
{
public:

    Window();

    Window(SIZE WindowSize, std::string_view WindowTitle);

    void ShowWindow();

    HWND GetWindowHandle() const;

    RECT GetWindowRect() const;

private:

    HWND WindowHandle {};

    void InitializeWindow(SIZE WindowRect, std::string_view WindowTitle);

    static LRESULT CALLBACK WindowProc(HWND Hwnd, UINT Message, WPARAM WParam, LPARAM LParam);
};
