#include "Runtime/Application/Windows/WindowsWindow.h"
#include "Runtime/Application/Core/Internal/WindowAllocator.h"
#include "SDL3/SDL_stdinc.h"
#include <glad/gl.h>

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

        SDL_SetPointerProperty(SDL_GetWindowProperties(Window), SLD_WINDOW_OBJECT_PROPERTY, this);

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

    bool WindowsWindow::CreateAndBindGlContext()
    {
        static SDL_GLContext Context {};

        if (!Context)
        {
            Context = SDL_GL_CreateContext(Window);

            if (!Context)
            {
                // TODO: log error
                return false;
            }

            if (gladLoadGL(SDL_GL_GetProcAddress) == 0)
            {
                // TODO: log error ("Failed to initialize OpenGL context")
                return false;
            }

            // TODO: log success

            return true;
        }

        // TODO: log success

        return SDL_GL_MakeCurrent(Window, Context);
    }

    WindowsWindow::~WindowsWindow()
    {
        // TODO: add log about closing the window in destructor

        // It is ok to call virtual here, we are final class
        Close();
    }

    glm::ivec2 WindowsWindow::GetSize() const
    {
        glm::ivec2 Size {};
        SDL_GetWindowSize(Window, &Size.x, &Size.y);

        return Size;
    }
}
