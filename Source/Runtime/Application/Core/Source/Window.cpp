#include "Runtime/Application/Core/Window.h"
#include "Runtime/Application/Core/Internal/WindowAllocator.h"

namespace krendrr::Runtime::Application::Core
{
    Window* Window::Create(const std::shared_ptr<RenderApi::Core::RenderApi>& RenderApi, const InitializeParams& Params)
    {
        Window* Window = ConstructWindowInstance();

        if (!Window)
        {
            // TODO: add error log about failed allocation
            return nullptr;
        }

        if (!Window->Initialize(RenderApi, Params))
        {
            // TODO: add error log about failed window init
            return nullptr;
        }

        // TODO: add success log

        return Window;
    }
}
