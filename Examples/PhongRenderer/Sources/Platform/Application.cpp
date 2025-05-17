#include "Application.h"

#include <chrono>
#include <iostream>

#include "Viewport.h"
#include "Sources/Render/Renderers/ForwardRenderer.h"
#include "Sources/World/World.h"
#include <Windows.h>

#include "Sources/Utils/Constexpr.h"
#include "Sources/Utils/Memory.h"
#include "Sources/Utils/Generators/MeshGenerator.h"

namespace kRendrr
{

    Application::Application()
        : ForwardRenderer(GetSharedPtrToStack(&RenderDevice), GetSharedPtrToStack(&CommandQueue)),
        World(GetSharedPtrToStack(&RenderDevice))
    {
    }

    void Application::Initialize()
    {
        RenderDevice.Initialize({
            .DebugMode = RenderDevice::RenderDeviceInitParams::DebugMode::Enabled
        });
        CommandQueue.Initialize(RenderDevice);
        Viewport.Initialize(RenderDevice, CommandQueue);
        ForwardRenderer.Initialize();
        World.Initialize();
    }

    void Application::GameLoop()
    {
        MSG Msg = {};

        double DeltaTime = 0.f;

        auto getCurrentTime = []
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();
        };

        uint64_t PrevTime = getCurrentTime();

        while(!bGotQuitEvent)
        {
            while(PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
            {
                if(Msg.message == WM_QUIT)
                {
                    RequestShutdown();

                    // use break since we want to quit as fast as possible
                    break;
                }

                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }

            if(bGotQuitEvent) {
                break;
            }

            World.Tick(DeltaTime);

            if(bGotQuitEvent) {
                break;
            }

            ForwardRenderer.Render(World, Viewport);

            uint64_t CurrentTime = getCurrentTime();

            uint64_t DeltaTimeMillis = CurrentTime - PrevTime;
            DeltaTime = static_cast<double>(DeltaTimeMillis) / 1000.;

            PrevTime = CurrentTime;
        }
    }

    void Application::Deinitialize()
    {

    }

    void Application::RequestShutdown()
    {
        bGotQuitEvent = true;
    }

}
