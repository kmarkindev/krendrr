#pragma once

#include <string_view>
#include "glm/vec2.hpp"

namespace krendrr::Runtime::Application::Core
{
    class Application;

    /**
     * Represents window in current platform.
     *
     * It is NOT required to close this window before Application shutdown call or window's destructor call.
     * So feel free to save it into smart pointer after Window::Create call and forget about it.
     *
     * Note that some platforms may only have one window, in this case window allocation will fail.
     *
     * If platform always has a window (like browsers do), you are going to get access to it by creating window for the first time.
     *
     * You can call initialize again, after Close was called
     */
    class Window
    {
    public:

        struct InitializeParams
        {
            std::string_view Title {};
            glm::ivec2 Size {};
        };

        explicit Window(Application* Application);

        [[nodiscard]] Application* GetApplication() const;

        virtual ~Window() = default;

        static Window* Create(Application* Application, const InitializeParams& Params);

        virtual bool Initialize(const InitializeParams& Params) = 0;

        virtual bool Close() = 0;

        /**
         * Checks if this object holds valid window.
         * It returns false after window was Closed or if this object was not initialized.
         */
        virtual bool IsValid() const = 0;

    private:

        Application* ParentApplication {};

    };
}

