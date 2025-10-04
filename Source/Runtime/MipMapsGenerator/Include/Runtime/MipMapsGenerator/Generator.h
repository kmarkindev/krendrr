#pragma once

#include <d3dx12/d3dx12.h>
#include "glm/vec2.hpp"
#include "Runtime/RenderApi/Core/RenderApi.h"

namespace krendrr::Runtime::MipMapsGenerator
{

/*
 * Holds state needed to run compute shader that generates mip maps for provided 2D texture resources.
 *
 * Note: Resources should be correctly allocated and setup for mip mapping before passing into generator.
 */
class Generator
{
public:

    struct TextureToProcess
    {
        ID3D12Resource* Resource {};
        DXGI_FORMAT Format {};
        unsigned MipZeroSize {};
        unsigned MipMapCount {};
        bool bShouldNormalize {};
    };

    /**
     * This pushes compute shader invocation per texture into compute queue.
     * Called should insert a fence and wait for completion before accessing passed texture resources or using this generator again.
     *
     * Expects texture to have a square size, and the size should the power of 2.
     */
    bool GenerateMipMaps(const RenderApi::Core::RenderApi& RenderApi, const std::span<TextureToProcess>& TexturesToProcess);

private:

    Microsoft::WRL::ComPtr<ID3D12PipelineState> ComputePipelineState {};
    Microsoft::WRL::ComPtr<ID3D12RootSignature> ComputeRootSignature {};

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> ComputeCommandAllocator {};
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> ComputeCommandList {};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ComputeGpuSrvUavDescriptorHeap {};
};

}
