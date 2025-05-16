#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include "Sources/Render/Resources/RenderResource.h"

namespace kRendrr
{
    class RenderDevice;

    class CommandQueue : public RenderResource
    {
    public:

        void Initialize(const RenderDevice& RenderDevice);

        [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetQueue() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> Queue {};

    };
}
