#include "UploadBuffer.h"

#include "Sources/Render/Resources/Commands/CommandList.h"
#include "Sources/Render/Resources/Sync/RenderFence.h"
#include "Sources/Utils/HResultCheck.h"
#include "Sources/Utils/Memory.h"

namespace kRendrr
{
    UploadBuffer::UploadBuffer()
    {
        BufferHeapType = D3D12_HEAP_TYPE_UPLOAD;
    }

    void UploadBuffer::UploadDataToBuffer(const RenderDevice& RenderDevice, CommandQueue& CommandQueue, DataBufferBase& TargetBuffer, size_t SizeBytes)
    {
        CommandAllocator CommandAllocator;
        CommandAllocator.Initialize(RenderDevice);

        CommandList CommandList {GetSharedPtrToStack(&CommandAllocator)};
        CommandList.Initialize(RenderDevice);

        RenderFence Fence {};
        Fence.Initialize(RenderDevice);

        CommandList.GetList()
                ->Reset(CommandAllocator.GetAllocator().Get(), nullptr)
                >> HResultCheck {};

        CommandList.GetList()
            ->CopyBufferRegion(
                TargetBuffer.GetBuffer().Get(),
                0,
                GetBuffer().Get(),
                0,
                SizeBytes
            );

        CommandList.GetList()
            ->Close()
            >> HResultCheck {};

        ID3D12CommandList* List[] = {CommandList.GetList().Get()};
        CommandQueue.GetQueue()
            ->ExecuteCommandLists(1, List);

        Fence.SignalQueue(CommandQueue);
        Fence.WaitSignaledValueSpinlock();
    }

    void UploadBuffer::UploadDataInternal(const void* Data, size_t Size)
    {
        CheckInitialization();

        if(Size > GetBufferSize())
        {
            throw std::runtime_error("Trying to upload more data than buffer can contain");
        }

        void* MappedPtr {};
        D3dBuffer->Map(0, nullptr, &MappedPtr)
            >> HResultCheck {};

        std::memcpy(MappedPtr, Data, Size);

        D3dBuffer->Unmap(0, nullptr);
    }
}

