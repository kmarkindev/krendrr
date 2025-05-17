#include "DataBufferBase.h"
#include <d3dx12/d3dx12_core.h>
#include "Sources/Render/RenderDevice.h"
#include "Sources/Utils/HResultCheck.h"

void kRendrr::DataBufferBase::Initialize(const RenderDevice& RenderDevice, std::uint64_t BufferSize)
{
    CheckInitialization(false);

    const auto HeapProps =  CD3DX12_HEAP_PROPERTIES(BufferHeapType);
    const auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);

    RenderDevice.GetDevice()
        ->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&D3dBuffer)
        ) >> HResultCheck{};

    MarkAsInitialized();
}

Microsoft::WRL::ComPtr<ID3D12Resource> kRendrr::DataBufferBase::GetBuffer() const
{
    CheckInitialization();

    return D3dBuffer;
}

std::uint64_t kRendrr::DataBufferBase::GetBufferSize() const
{
    CheckInitialization();

    return D3dBuffer->GetDesc().Width;
}
