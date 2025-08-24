#include "Runtime/Application/Core/Window.h"
#include "Runtime/Application/Core/Internal/WindowAllocator.h"

namespace krendrr::Runtime::Application::Core
{
    Window::Window(Application* Application)
        : ParentApplication(Application)
    {
    }

    Application* Window::GetApplication() const
    {
        return ParentApplication;
    }

    Window* Window::Create(Application* Application, const InitializeParams& Params)
    {
        Window* Window = ConstructWindowInstance(Application);

        if (!Window)
        {
            // TODO: add error log about failed allocation
            return nullptr;
        }

        if (!Window->Initialize(Params))
        {
            // TODO: add error log about failed window init
            return nullptr;
        }

        // TODO: add success log

        return Window;
    }
}
