#include "UploadBuffer.h"

#include "Sources/Render/Resources/Commands/CommandList.h"
#include "Sources/Utils/HResultCheck.h"

namespace kRendrr
{
    UploadBuffer::UploadBuffer()
    {
        BufferHeapType = D3D12_HEAP_TYPE_UPLOAD;
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

