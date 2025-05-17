#pragma once

#include <memory>

namespace kRendrr
{
    class RenderDevice;

    class World
    {
    public:

        explicit World(std::shared_ptr<RenderDevice> RenderDevice);

        void Tick(double DeltaTime);

        void Initialize();

        void Uninitialize();

    private:

        std::shared_ptr<RenderDevice> RenderDevice;

    };

}
