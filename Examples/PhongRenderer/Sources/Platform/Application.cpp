#include "Application.h"
#include "Viewport.h"
#include "Sources/Render/RenderDevice.h"
#include "Sources/Render/Renderers/ForwardRenderer.h"
#include "Sources/World/World.h"
#include <Windows.h>

void Application::Initialize()
{
    RenderDevice = std::make_shared<::RenderDevice>();
    Viewport = std::make_shared<::Viewport>(RenderDevice);
    World = std::make_shared<::World>(RenderDevice);
    ForwardRenderer = std::make_shared<::ForwardRenderer>(RenderDevice);

    RenderDevice->Initialize();
    Viewport->Initialize();
    World->Initialize();
    ForwardRenderer->Initialize();
}

void Application::GameLoop()
{
    while(!bGotQuitEvent)
    {
        MSG Msg = {};

        while (!bGotQuitEvent && PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
        {
            if(Msg.message == WM_QUIT)
            {
                RequestShutdown();
            }

            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }

        if(bGotQuitEvent)
        {
            break;
        }

        World->Tick();

        if(bGotQuitEvent)
        {
            break;
        }

        ForwardRenderer->Render(*World, *Viewport);
    }
}

void Application::Deinitialize()
{

}

void Application::RequestShutdown()
{
    bGotQuitEvent = true;
}
