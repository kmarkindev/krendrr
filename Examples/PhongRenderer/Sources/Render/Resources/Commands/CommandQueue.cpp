#include "CommandQueue.h"
#include "Sources/Render/RenderDevice.h"
#include "Sources/Utils/HResultCheck.h"

namespace kRendrr
{
    void CommandQueue::Initialize(const RenderDevice& RenderDevice)
    {
        CheckInitialization(false);

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        RenderDevice.GetDevice()
            ->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&Queue))
            >> HResultCheck {};

        MarkAsInitialized();
    }

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue::GetQueue() const
    {
        CheckInitialization();

        return Queue;
    }
}