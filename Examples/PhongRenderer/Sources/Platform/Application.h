#pragma once

#include "Sources/Render/Renderers/ForwardRenderer.h"
#include "Sources/World/World.h"
#include "Viewport.h"

namespace kRendrr
{
    class Application
    {
    public:

        Application();

        virtual ~Application() = default;

        virtual void Initialize();

        virtual void GameLoop();

        virtual void Deinitialize();

        virtual void RequestShutdown();

    private:

        bool bGotQuitEvent {false};

        RenderDevice RenderDevice;
        CommandQueue CommandQueue;
        Viewport Viewport;
        ForwardRenderer ForwardRenderer;
        World World;
    };

}
