#pragma once

namespace krendrr::Runtime::Application::Core
{
    class Application;
    class Window;

    Window* ConstructWindowInstance(Application* Application);
}

/**
 * This is called by platform specific implementation
 */
#define IMPLEMENT_WINDOW_ALLOCATOR(WindowType) \
namespace krendrr::Runtime::Application::Core \
{ \
    Window* ConstructWindowInstance(Application* Application) \
    { \
        return new WindowType {Application}; \
    } \
} \
