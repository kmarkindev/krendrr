#pragma once

#include "Runtime/Application/Core/Window.h"
#include "SDL3/SDL_video.h"

namespace krendrr::Runtime::Application::Windows
{
    class WindowsWindow final : public Core::Window
    {
    public:

        explicit WindowsWindow(Core::Application* Application);

        bool Initialize(const InitializeParams& Params) override;

        bool Close() override;

        bool IsValid() const override;

        ~WindowsWindow() override;

    private:

        SDL_Window* Window {};

    };
}

