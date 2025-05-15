#pragma once
#include <memory>


class RenderDevice;

class World
{
public:

    explicit World(std::shared_ptr<RenderDevice> RenderDevice);

    void Tick();

    void Initialize();

    void Uninitialize();

private:

    std::shared_ptr<RenderDevice> RenderDevice;

};
