#pragma once
#include "Events/QuitEvent.h"

namespace krendrr::Runtime::Application::Core
{
    class KeyEvent;
    class MouseMoveEvent;
    class MouseWheelEvent;
    class StartupArgs;

    /**
     * Base class for all krendrr applications.
     *
     * Override virtual functions to make you App do something.
     * Note that some virtual functions are protected and not called directly from platform specific code.
     *
     * Use the following macro from EntryPoint.h to inject it into platform specific code:
     * IMPLEMENT_ENTRY_POINT(YourInheritedClass)
     *
     * Base Application class does nothing and just ticks infinitely.
     */
    class Application
    {
    public:

        explicit Application(const StartupArgs& Args);

        /**
         * Called by platform specific code as fast as possible.
         *
         * Returning false means there was an error, and we need to shut down
         */
        virtual bool Tick(float DeltaTime);

        /**
         * Called one time by platform specific code before any Tick calls
         *
         * Returning false means there was an error, and we need to shut down
         */
        virtual bool Initialize();

        /**
         * Called by platform specific code after there was an error or shutdown request.
         * In some cases, platform may decide to shut down the application, so it is going to be called as well.
         *
         * Returning false means there was an error, and we need to shut down
         */
        virtual bool Shutdown();

        /**
         * Can be called any time to make a request to shut down this application.
         * It is going to shut down after current tick was finished, if any.
         * Note: Presence of shutdown request is checked by platform specific code.
         */
        void RequestShutdown();

        [[nodiscard]] bool HasRequestedShutdown() const;

        virtual void HandleKeyEvent(const KeyEvent& Event);

        virtual void HandleMouseMoveEvent(const MouseMoveEvent& Event);

        virtual void HandleMouseWheelEvent(const MouseWheelEvent& Event);

        virtual void HandleQuitEvent(const QuitEvent& Event);

        virtual ~Application() = default;

    private:

        bool bHasRequestShutdown {};

    };
}

