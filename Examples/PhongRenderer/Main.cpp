#include <exception>
#include <iostream>
#include <csignal>
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

bool bGotQuitEvent = false;

void HandleException(std::string_view Error)
{
    std::cerr << "Error: " << Error << '\n';
}

void SignalHandler(int Signal)
{
    switch (Signal)
    {
        case SIGTERM:
            HandleException("SIGINT (signal)");
            break;
        case SIGSEGV:
            HandleException("Invalid memory access (signal)");
            break;
        case SIGINT:
            HandleException("External interrupt, all unsaved data is lost (signal)");
            break;
        case SIGILL:
            HandleException("Invalid program image (signal)");
            break;
        case SIGABRT:
            HandleException("Internal abort, all unsaved data is lost (signal)");
            break;
        case SIGFPE:
            HandleException("FPE error, probably division by 0 (signal)");
            break;
        default:
            HandleException("Unknown error (signal)");
            break;
    }
}

void SetupSignals()
{
    std::signal(SIGTERM, SignalHandler);
    std::signal(SIGSEGV, SignalHandler);
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGILL, SignalHandler);
    std::signal(SIGABRT, SignalHandler);
    std::signal(SIGFPE, SignalHandler);
}

void ProcessEvents()
{
    MSG Msg = {};

    while (!bGotQuitEvent && PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
    {
        bGotQuitEvent = Msg.message == WM_QUIT;

        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
}

int main() try
{
    SetupSignals();

    Window Window {};
    RenderDevice RenderDevice {};
    SwapChain SwapChain {RenderDevice, Window};
    World World {RenderDevice};
    ForwardRenderer ForwardRenderer {RenderDevice};

    Window.Initialize();
    World.Initialize();
    RenderDevice.Initialize();
    SwapChain.Initialize();

    while(!bGotQuitEvent)
    {
        ProcessEvents();

        if(bGotQuitEvent)
        {
            break;
        }

        World.Tick();
        ForwardRenderer.Render(World, SwapChain);
    }

    RenderDevice.Uninitialize();
    Window.Uninitialize();
    World.Uninitialize();
    SwapChain.Uninitialize();

    return 0;
}
catch(const std::exception& e)
{
    HandleException(e.what());
}
catch (...)
{
    HandleException("Unknown exception");
}