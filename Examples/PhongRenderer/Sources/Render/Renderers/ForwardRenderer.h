#pragma once

#include <memory>
#include "Sources/Render/Resources/Buffers/DescriptorHeap.h"
#include "Sources/Render/Resources/Buffers/UploadBuffer.h"
#include "Sources/Render/Resources/Commands/CommandList.h"
#include "Sources/Render/Resources/Geometry/IndexBuffer.h"
#include "Sources/Render/Resources/Geometry/VertexBuffer.h"
#include "Sources/Render/Resources/Pipeline/PipelineStateObject.h"
#include "Sources/Render/Resources/Pipeline/RootSignature.h"
#include "Sources/Render/Resources/Shaders/Shader.h"
#include "Sources/Render/Resources/Sync/RenderFence.h"
#include "Sources/Render/Resources/Texture/Texture.h"

namespace kRendrr
{
    class DescriptorHeap;
}

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

        void Initialize();

        void Render(const World& World, Viewport& Viewport);

    private:

        std::shared_ptr<RenderDevice> RenderDevice {};
        std::shared_ptr<CommandQueue> CommandQueue {};

        CommandAllocator CommandAllocator {};
        CommandList CommandList;
        RenderFence Fence {};

        VertexBuffer MeshVertexBuffer {};
        IndexBuffer MeshIndexBuffer {};
        UploadBuffer MeshUploadBuffer {};

        Shader MeshVertexShader {};
        Shader MeshPixelShader {};

        RootSignature MeshRootSignature {};
        PipelineStateObject MeshPso;

        Texture MeshTexture {};

        Texture DepthStencilTexture {};
        glm::ivec2 DepthStencilSize {};
        DescriptorHeap DepthStencilHeap {};

        DescriptorHeap RenderSrvDescriptorHeap {};

        void InitializeDepthStencil(glm::ivec2 Size);
    };
}
