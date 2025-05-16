#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>
#include "Sources/Render/Resources/RenderResource.h"

namespace kRendrr
{
    class CommandQueue;
    class RenderDevice;

    class RenderFence : public RenderResource
    {
    public:

        void Initialize(const RenderDevice& RenderDevice);

        /**
         * Sends fence into specified queue with specified value.
         *
         * Use any Wait* function to wait on this fence,
         * or quickly check if it has been reached using HasReachedSignaledValue.
         */
        void SignalQueue(const CommandQueue& Queue);

        void WaitSignaledValueSleep() const;

        void WaitSignaledValueSpinlock() const;

        bool HasReachedSignaledValue() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12Fence> D3dFence {};
        HANDLE FenceEvent {};
        uint64_t SignaledFenceValue {};

    };
}
