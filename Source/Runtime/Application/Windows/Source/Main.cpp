#include "Runtime/Application/Core/Application.h"
#include "Runtime/Application/Core/EntryPoint.h"
#include "Runtime/Application/Core/StartupArgs.h"

int main(int Argc, char** Argv)
{
    krendrr::Runtime::Application::Core::StartupArgs StartupArgs {Argc, Argv};
    krendrr::Runtime::Application::Core::Application* Application = krendrr::Runtime::Application::Core::ConstructApplicationInstance(StartupArgs);

    if (!Application->Initialize())
        return -1;

    bool bHasTickFailed {};
    while (!bHasTickFailed && !Application->HasRequestedShutdown())
    {
        bHasTickFailed = !Application->Tick();
    }

    if (!Application->Shutdown())
        return -1;

    return bHasTickFailed ? -1 : 0;
}
