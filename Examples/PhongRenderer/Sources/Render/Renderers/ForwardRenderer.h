#pragma once

#include <memory>

class RenderDevice;
class World;
class Viewport;

class ForwardRenderer
{
public:

    explicit ForwardRenderer(std::shared_ptr<RenderDevice> RenderDevice);

    void Render(const World& World, const Viewport& Viewport);

    void Initialize();

    void Uninitialize();

private:

    std::shared_ptr<RenderDevice> RenderDevice {};

};
