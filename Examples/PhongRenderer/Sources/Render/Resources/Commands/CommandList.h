#pragma once

#include <memory>
#include "CommandAllocator.h"
#include "Sources/Render/Resources/RenderResource.h"

namespace kRendrr
{
    class CommandList : public RenderResource
    {
    public:

        explicit CommandList(std::shared_ptr<CommandAllocator> CommandAllocator);

        void Initialize(const RenderDevice& RenderDevice);

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetList() const;

    private:

        std::shared_ptr<CommandAllocator> CommandAllocator;

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3dCommandList {};

    };
}
