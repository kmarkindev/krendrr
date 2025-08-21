#include "Application.h"
#include <iostream>
#include <memory>
#include "Runtime/Application/Core/EntryPoint.h"
#include "Runtime/Application/Core/Window.h"
#include "Runtime/Application/Core/Events/KeyEvent.h"

IMPLEMENT_ENTRY_POINT(krendrr::Examples::SimpleDeferredRendering::Application)

krendrr::Examples::SimpleDeferredRendering::Application::Application(const Runtime::Application::Core::StartupArgs& Args)
    : Runtime::Application::Core::Application(Args)
{

}

bool krendrr::Examples::SimpleDeferredRendering::Application::Initialize()
{
    Window = std::unique_ptr<Runtime::Application::Core::Window>{
        Runtime::Application::Core::Window::Create(this, {
            .Title = "Deferred Rendering Example",
            .Size = {1280, 720}
        })
    };

    return true;
}

bool krendrr::Examples::SimpleDeferredRendering::Application::Tick()
{
    return true;
}

bool krendrr::Examples::SimpleDeferredRendering::Application::Shutdown()
{
    return true;
}
