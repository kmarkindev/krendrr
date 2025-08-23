#pragma once

#include <memory>
#include "Runtime/Application/Core/Application.h"
#include "Runtime/Renderer/Deferred/DeferredRenderer.h"

namespace krendrr::Runtime::Application::Core
{
    class Window;
}

namespace krendrr::Examples::SimpleDeferredRendering
{
    class Application final : public Runtime::Application::Core::Application
    {
    public:

        explicit Application(const Runtime::Application::Core::StartupArgs& Args);

        bool Tick() override;

        bool Initialize() override;

        bool Shutdown() override;

    private:

        std::unique_ptr<Runtime::Application::Core::Window> Window {};
        std::unique_ptr<Runtime::Renderer::Core::Scene> Scene {};
        std::unique_ptr<Runtime::Renderer::Core::Renderer> Renderer {};

    };
}


