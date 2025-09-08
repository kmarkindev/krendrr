#pragma once

namespace krendrr::Runtime::Application::Core
{
    class Application;
    class Window;

    Window* ConstructWindowInstance();
}

/**
 * This is called by platform specific implementation
 */
#define IMPLEMENT_WINDOW_ALLOCATOR(WindowType) \
namespace krendrr::Runtime::Application::Core \
{ \
    Window* ConstructWindowInstance() \
    { \
        return new WindowType {}; \
    } \
} \
