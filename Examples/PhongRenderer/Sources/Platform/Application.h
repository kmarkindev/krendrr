#pragma once
#include <memory>

class ForwardRenderer;
class World;
class Viewport;
class RenderDevice;

class Application
{
public:

    virtual ~Application() = default;

    virtual void Initialize();

    virtual void GameLoop();

    virtual void Deinitialize();

    virtual void RequestShutdown();

private:

    bool bGotQuitEvent {false};

    std::shared_ptr<RenderDevice> RenderDevice {};
    std::shared_ptr<ForwardRenderer> ForwardRenderer {};
    std::shared_ptr<Viewport> Viewport {};
    std::shared_ptr<World> World {};
};
