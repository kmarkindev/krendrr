#include "DescriptorHeap.h"
#include "Sources/Render/RenderDevice.h"
#include "Sources/Utils/HResultCheck.h"

void kRendrr::DescriptorHeap::Initialize(const RenderDevice& RenderDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type, std::uint32_t DescriptorsCount, bool bShaderVisible)
{
    CheckInitialization(false);

    this->bShaderVisible = bShaderVisible;

    const D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
        .Type = Type,
        .NumDescriptors = DescriptorsCount,
        .Flags = bShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    };

    RenderDevice.GetDevice()
        ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&D3dDescriptorHeap))
        >> HResultCheck {};

    MarkAsInitialized();
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> kRendrr::DescriptorHeap::GetDescriptorHeap() const
{
    CheckInitialization();

    return D3dDescriptorHeap;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE kRendrr::DescriptorHeap::GetCPUHandle(std::int32_t Index) const
{
    CheckInitialization();

    CD3DX12_CPU_DESCRIPTOR_HANDLE Handle {
        D3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
    };

    return Handle.Offset(Index);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE kRendrr::DescriptorHeap::GetGPUHandle(std::int32_t Index) const
{
    CheckInitialization();

    if(!bShaderVisible)
    {
        throw std::runtime_error("Trying to get GPU handle from non shader visible descriptor heap");
    }

    CD3DX12_GPU_DESCRIPTOR_HANDLE Handle {
        D3dDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
    };

    return Handle.Offset(Index);
}
