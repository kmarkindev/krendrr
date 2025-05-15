#include "ForwardRenderer.h"

ForwardRenderer::ForwardRenderer(std::shared_ptr<::RenderDevice> RenderDevice)
    : RenderDevice(std::move(RenderDevice))
{

}

void ForwardRenderer::Render(const World& World, const Viewport& Viewport)
{
}

void ForwardRenderer::Initialize()
{

}

void ForwardRenderer::Uninitialize()
{

}
