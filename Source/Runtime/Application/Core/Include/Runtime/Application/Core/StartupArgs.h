#pragma once
#include <span>
#include <string_view>
#include <vector>

namespace krendrr::Runtime::Application::Core
{
    class StartupArgs
    {
    public:

        StartupArgs() = default;

        StartupArgs(int Argc, char** Argv);

        std::span<const std::string_view> GetStartupArgs() const;

    private:

        std::vector<std::string_view> StoredArgs {};

    };
}

