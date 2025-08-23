#pragma once

#include "Runtime/Application/Core/Window.h"
#include "SDL3/SDL_video.h"

namespace krendrr::Runtime::Application::Windows
{
    class WindowsWindow final : public Core::Window
    {
    public:

        constexpr static const char* SLD_WINDOW_OBJECT_PROPERTY = "window_object";

        explicit WindowsWindow(Core::Application* Application);

        bool Initialize(const InitializeParams& Params) override;

        bool Close() override;

        [[nodiscard]] bool IsValid() const override;

        [[nodiscard]] bool CreateAndBindGlContext() override;

        ~WindowsWindow() override;

    private:

        SDL_Window* Window {};

    };
}

