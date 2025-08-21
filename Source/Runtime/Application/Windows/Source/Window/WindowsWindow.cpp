#include "Runtime/Application/Windows/WindowsWindow.h"
#include "Runtime/Application/Core/Internal/WindowAllocator.h"
#include "SDL3/SDL_stdinc.h"

IMPLEMENT_WINDOW_ALLOCATOR(krendrr::Runtime::Application::Windows::WindowsWindow)

namespace krendrr::Runtime::Application::Windows
{
    WindowsWindow::WindowsWindow(Core::Application* Application)
        : Core::Window(Application)
    {

    }

    bool WindowsWindow::Initialize(const InitializeParams& Params)
    {
        if (IsValid())
        {
            // TODO: add error log
            return false;
        }

        Window = SDL_CreateWindow(
            Params.Title.data(),
            Params.Size.x,
            Params.Size.y,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS
        );

        if (Window == nullptr)
        {
            // TODO: add error log
            return false;
        }

        // TODO: add success log

        return true;
    }

    bool WindowsWindow::Close()
    {
        if (!IsValid())
        {
            // TODO: add error log
            return false;
        }

        SDL_DestroyWindow(Window);
        Window = nullptr;

        // TODO: add success log

        return true;
    }

    bool WindowsWindow::IsValid() const
    {
        return Window != nullptr;
    }

    WindowsWindow::~WindowsWindow()
    {
        // TODO: add log about closing the window in destructor

        // It is ok to call virtual here, we are final class
        Close();
    }
}
