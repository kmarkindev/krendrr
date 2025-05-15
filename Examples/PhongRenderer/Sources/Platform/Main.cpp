#include <exception>
#include <iostream>
#include <csignal>
#include "Application.h"

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

int main() try
{
    SetupSignals();

    Application Application {};
    Application.Initialize();
    Application.GameLoop();
    Application.Deinitialize();

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