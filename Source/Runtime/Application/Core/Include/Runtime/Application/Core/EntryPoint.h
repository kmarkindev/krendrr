#pragma once

namespace krendrr::Runtime::Application::Core
{
    class Application;
    class StartupArgs;

    Application* ConstructApplicationInstance(const StartupArgs& Args);
}

/**
 * Create your App class based from krendrr::Runtime::Application::Core::Application, then provide it as ApplicationType
 * using this macro to inject it into platform specific application implementation.
 *
 * You can paste this macro in any .cpp file you want. e.g. inside separate .cpp file or inside you application.cpp file.
 * But, this should only be used once, otherwise there is going to be a break in One Definition Rule and compilation error as a result.
 *
 * You do NOT need to implement main() entry point. It is already implemented inside platform-specific code and provided to you
 * when you link to krendrr::runtime::application CMake target.
 *
 * @param ApplicationType Type of application to use as an entry point.
 */
#define IMPLEMENT_ENTRY_POINT(ApplicationType) \
namespace krendrr::Runtime::Application::Core \
{ \
    Application* ConstructApplicationInstance(const StartupArgs& Args) \
    { \
        return new ApplicationType {Args}; \
    } \
} \
