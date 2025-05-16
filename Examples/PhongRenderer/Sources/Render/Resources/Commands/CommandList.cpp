#include "CommandList.h"

#include "Sources/Utils/HResultCheck.h"

namespace kRendrr
{
    CommandList::CommandList(std::shared_ptr<kRendrr::CommandAllocator> CommandAllocator)
        : CommandAllocator(std::move(CommandAllocator))
    {
    }

    void CommandList::Initialize(const RenderDevice& RenderDevice)
    {
        CheckInitialization(false);

        RenderDevice.GetDevice()
            ->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                CommandAllocator->GetAllocator().Get(),
                nullptr,
                IID_PPV_ARGS(&D3dCommandList)
            )
            >> HResultCheck {};

        D3dCommandList->Close()
            >> HResultCheck {};

        MarkAsInitialized();
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList::GetList() const
    {
        CheckInitialization();
        return D3dCommandList;
    }

}