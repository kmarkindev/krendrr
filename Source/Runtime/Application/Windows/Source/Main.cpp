#include <memory>

#include "Runtime/Application/Core/Application.h"
#include "Runtime/Application/Core/EntryPoint.h"
#include "Runtime/Application/Core/StartupArgs.h"
#include "SDL3/SDL_events.h"

int main(int Argc, char** Argv)
{
    krendrr::Runtime::Application::Core::StartupArgs StartupArgs {Argc, Argv};

    std::unique_ptr<krendrr::Runtime::Application::Core::Application> Application { krendrr::Runtime::Application::Core::ConstructApplicationInstance(StartupArgs) };

    if (!Application->Initialize())
        return -1;

    bool bHasTickFailed {};
    while (!bHasTickFailed && !Application->HasRequestedShutdown())
    {

        SDL_Event Event {};
        while (SDL_PollEvent(&Event)) {

        }

        bHasTickFailed = !Application->Tick();
    }

    if (!Application->Shutdown())
        return -1;

    return bHasTickFailed ? -1 : 0;
}
