#include "Runtime/Application/Core/StartupArgs.h"

krendrr::Runtime::Application::Core::StartupArgs::StartupArgs(int Argc, char** Argv)
    : StoredArgs(Argv, Argv + Argc)
{

}

std::span<const std::string_view> krendrr::Runtime::Application::Core::StartupArgs::GetStartupArgs() const
{
    return StoredArgs;
}
