#pragma once

#include "Runtime/RenderApi/Core/ApiCallCheck.h"
#include "Runtime/RenderApi/Core/RenderApi.h"

namespace krendrr::Runtime::RenderApi::Core
{

    template<typename ConstBufferType>
    bool BuildConstantBuffer(
        const RenderApi& RenderApi,
        Microsoft::WRL::ComPtr<ID3D12Resource>& ConstantBuffer,
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& DescriptorHeap,
        const std::wstring_view& ConstantBufferName = L"Constant Buffer"
        )
    {
        static_assert(alignof(ConstBufferType) == 256);

        const CD3DX12_HEAP_PROPERTIES HeapProps {D3D12_HEAP_TYPE_UPLOAD};
        const CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstBufferType));

        CHECKED(
            RenderApi.GetDevice()
                ->CreateCommittedResource(
                    &HeapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &ResourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&ConstantBuffer)
                ),
            "Can't create constant buffer"
        )

        ConstantBuffer->SetName(ConstantBufferName.data());

        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };

        CHECKED(
            RenderApi.GetDevice()
                ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeap)),
            "Failed to create srv descriptor heap"
        )

        D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc {
            .BufferLocation = ConstantBuffer->GetGPUVirtualAddress(),
            .SizeInBytes = sizeof(ConstBufferType)
        };

        RenderApi.GetDevice()
            ->CreateConstantBufferView(&CbvDesc, DescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        return true;
    }

}
