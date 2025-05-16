#include "RenderFence.h"

#include "Sources/Render/RenderDevice.h"
#include "Sources/Render/Resources/Commands/CommandQueue.h"
#include "Sources/Utils/HResultCheck.h"

namespace kRendrr
{
    void RenderFence::Initialize(const RenderDevice& RenderDevice)
    {
        CheckInitialization(false);

        RenderDevice.GetDevice()
            ->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&D3dFence))
            >> HResultCheck {};

        FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        MarkAsInitialized();
    }

    void RenderFence::SignalQueue(const CommandQueue& Queue)
    {
        CheckInitialization();

        SignaledFenceValue++;

        Queue.GetQueue()
            ->Signal(D3dFence.Get(), SignaledFenceValue)
            >> HResultCheck {};
    }

    void RenderFence::WaitSignaledValueSleep() const
    {
        CheckInitialization();

        if(HasReachedSignaledValue())
        {
            return;
        }

        D3dFence->SetEventOnCompletion(SignaledFenceValue, FenceEvent)
            >> HResultCheck {};

        ::WaitForSingleObject(FenceEvent, INFINITE);
    }

    void RenderFence::WaitSignaledValueSpinlock() const
    {
        CheckInitialization();

        while(!HasReachedSignaledValue());
    }

    bool RenderFence::HasReachedSignaledValue() const
    {
        CheckInitialization();

        return D3dFence->GetCompletedValue() >= SignaledFenceValue;
    }
}
