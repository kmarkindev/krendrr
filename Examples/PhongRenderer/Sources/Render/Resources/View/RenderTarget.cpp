#include "RenderTarget.h"
#include <stdexcept>
#include <utility>

namespace kRendrr
{

    void RenderTarget::Initialize(const RenderDevice& RenderDevice)
    {
        // TODO: Allow to create our own RT, not attached to swap chain
        // also, we may not want to store cpu handle in RT object...
        // it's probably better to get it from Swap Chain since it owns Descriptor Heap.

        throw std::runtime_error("Not implemented");
    }

    void RenderTarget::Initialize(Microsoft::WRL::ComPtr<ID3D12Resource> RtvResource, D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle)
    {
        Resource = std::move(RtvResource);
        this->CpuHandle = CpuHandle;
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> RenderTarget::GetResource() const
    {
        return Resource;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget::GetCpuHandle() const
    {
        return CpuHandle;
    }
}
