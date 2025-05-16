#include "CommandAllocator.h"
#include "Sources/Utils/HResultCheck.h"

namespace kRendrr
{
    void CommandAllocator::Initialize(const RenderDevice& RenderDevice)
    {
        CheckInitialization(false);

        RenderDevice.GetDevice()
            ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&D3dCommandAllocator))
            >> HResultCheck {};

        MarkAsInitialized();
    }

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator::GetAllocator() const
    {
        CheckInitialization();

        return D3dCommandAllocator;
    }
}

