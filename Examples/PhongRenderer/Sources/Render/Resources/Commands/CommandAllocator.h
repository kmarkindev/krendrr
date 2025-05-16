#pragma once

#include "CommandQueue.h"
#include "Sources/Render/RenderDevice.h"
#include "Sources/Render/Resources/RenderResource.h"

namespace kRendrr
{
    class CommandAllocator : public RenderResource
    {
    public:

        void Initialize(const RenderDevice& RenderDevice);

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetAllocator() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> D3dCommandAllocator {};

    };
}
