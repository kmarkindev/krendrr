#include "Runtime/Application/Core/Application.h"

namespace krendrr::Runtime::Application::Core
{
    Application::Application(const StartupArgs& Args)
    {
    }

    bool Application::Tick()
    {
        return true;
    }

    bool Application::Initialize()
    {
        return true;
    }

    bool Application::Shutdown()
    {
        return true;
    }

    void Application::RequestShutdown()
    {
        bHasRequestShutdown = true;
    }

    bool Application::HasRequestedShutdown() const
    {
        return bHasRequestShutdown;
    }

    void Application::HandleKeyEvent(const KeyEvent& Event)
    {
    }

    void Application::HandleMouseMoveEvent(const MouseMoveEvent& Event)
    {
    }

    void Application::HandleMouseWheelEvent(const MouseWheelEvent& Event)
    {
    }

    void Application::HandleQuitEvent(const QuitEvent& Event)
    {
        RequestShutdown();
    }
}

