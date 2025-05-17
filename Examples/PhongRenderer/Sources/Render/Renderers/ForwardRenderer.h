#pragma once

#include <memory>

#include "Sources/Render/Resources/Buffers/UploadBuffer.h"
#include "Sources/Render/Resources/Commands/CommandList.h"
#include "Sources/Render/Resources/Geometry/IndexBuffer.h"
#include "Sources/Render/Resources/Geometry/VertexBuffer.h"
#include "Sources/Render/Resources/Sync/RenderFence.h"

namespace kRendrr
{
    class CommandQueue;
    class RenderDevice;
    class World;
    class Viewport;

    class ForwardRenderer
    {
    public:

        explicit ForwardRenderer(std::shared_ptr<RenderDevice> RenderDevice, std::shared_ptr<CommandQueue> CommandQueue);

        void Render(const World& World, Viewport& Viewport);

        void Initialize();

    private:

        std::shared_ptr<RenderDevice> RenderDevice {};
        std::shared_ptr<CommandQueue> CommandQueue {};

        CommandAllocator CommandAllocator {};
        CommandList CommandList;
        RenderFence Fence {};

        VertexBuffer CubeVertexBuffer {};
        IndexBuffer CubeIndexBuffer {};
        UploadBuffer CubeUploadBuffer {};

    };
}
