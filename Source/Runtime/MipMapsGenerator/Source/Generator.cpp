#define NOMINMAX

#include "Runtime/MipMapsGenerator/Generator.h"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"
#include <d3dcompiler.h>
#include <algorithm>

namespace krendrr::Runtime::MipMapsGenerator
{

    bool Generator::GenerateMipMaps(RenderApi::Core::RenderApi& RenderApi, const std::span<TextureToProcess>& TexturesToProcess)
    {
        // Allocate everything if not allocated
        if (!ComputePipelineState)
        {
            CD3DX12_ROOT_PARAMETER RootParams[3] {};

            CD3DX12_DESCRIPTOR_RANGE Ranges[2] {};
            Ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            Ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

            RootParams[0].InitAsDescriptorTable(2, Ranges);
            RootParams[1].InitAsConstants(3, 0);

            CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc {};
            RootSignatureDesc.Init(
                std::size(RootParams),
                RootParams,
                0,
                nullptr
            );

            Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureBlob {};
            Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrorBlob {};

            HRESULT RootSigSerResult = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &RootSignatureBlob, &RootSignatureErrorBlob);
            if (FAILED(RootSigSerResult))
            {
                std::string error (static_cast<const char*>(RootSignatureErrorBlob->GetBufferPointer()), RootSignatureErrorBlob->GetBufferSize());
                __debugbreak();
                return false;
            }

            CHECKED(
                RenderApi.GetDevice()
                    ->CreateRootSignature(
                        0,
                        RootSignatureBlob->GetBufferPointer(),
                        RootSignatureBlob->GetBufferSize(),
                        IID_PPV_ARGS(&ComputeRootSignature)
                    ),
                "Can't create root signature"
            )

                Microsoft::WRL::ComPtr<ID3DBlob> Shader {};
            {
                {
                    Microsoft::WRL::ComPtr<ID3DBlob> CompilationErrorBlob {};

                    HRESULT VSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_mip_maps_generator/Shaders/GenerateMips.hlsl",
                        nullptr, nullptr, "Main", "vs_5_1",
                        RenderApi.GetShaderCompileFlags(), 0, &Shader, &CompilationErrorBlob);

                    if(FAILED(VSCompileResult) || CompilationErrorBlob != nullptr)
                    {
                        std::string error( static_cast<char*>(CompilationErrorBlob->GetBufferPointer()), CompilationErrorBlob->GetBufferSize());
                        // TODO: log error

                        __debugbreak();

                        return false;
                    }
                }
            }

            D3D12_COMPUTE_PIPELINE_STATE_DESC PsoDesc {
                .pRootSignature = ComputeRootSignature.Get(),
                .CS = CD3DX12_SHADER_BYTECODE {},
            };

            CHECKED(
                RenderApi.GetDevice()
                    ->CreateComputePipelineState(&PsoDesc, IID_PPV_ARGS(&ComputePipelineState)),
                "Can't create compute pipeline"
            )

            CHECKED(
                RenderApi.GetDevice()
                    ->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&ComputeCommandAllocator)),
                "Failed to create command allocator"
            )

            CHECKED(
                RenderApi.GetDevice()
                    ->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE,
                        ComputeCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&ComputeCommandList)),
                "Failed to create command list"
            )

            CHECKED_S(ComputeCommandList->Close());
        }

        const unsigned DescriptorOffset = RenderApi.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // Create descriptors
        {
            // Allocate enough space by figuring out how many shader invocation we need
            {
                unsigned InvocationsCount = 0;

                for (int i = 0; i < TexturesToProcess.size(); i++)
                {
                    const TextureToProcess& Texture = TexturesToProcess[i];
                    InvocationsCount += Texture.MipMapCount;
                }

                if (InvocationsCount == 0)
                {
                    // TODO: log error
                    return false;
                }

                D3D12_DESCRIPTOR_HEAP_DESC HeapDesc {
                    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                    .NumDescriptors = InvocationsCount * 2, // one SRV and UAV handle for each invocation
                    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                };

                CHECKED(
                    RenderApi.GetDevice()
                        ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&ComputeGpuSrvUavDescriptorHeap)),
                    "Could not create descriptor heap"
                )
            }

            // Then fill descriptors, texture by texture, mip level by mip level
            for (int i = 0; i < TexturesToProcess.size(); i++)
            {
                const TextureToProcess& Texture = TexturesToProcess[i];

                if (TexturesToProcess[i].MipMapCount == 0)
                {
                    // TODO: log error "invalid input. specify count > 0 to generate mipmaps"
                    return false;
                }

                for (unsigned mipIndex = 0; mipIndex < TexturesToProcess[i].MipMapCount - 1; mipIndex++)
                {
                    D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc = CD3DX12_UNORDERED_ACCESS_VIEW_DESC::Tex2D(Texture.Format, mipIndex + 1);

                    CD3DX12_CPU_DESCRIPTOR_HANDLE Handle {
                        ComputeGpuSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                        i * 2,
                        DescriptorOffset
                    };

                    RenderApi.GetDevice()
                        ->CreateUnorderedAccessView(Texture.Resource, nullptr, &UavDesc, Handle);

                    D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2D(Texture.Format, 1, 0);

                    Handle.Offset(1, DescriptorOffset);

                    RenderApi.GetDevice()
                        ->CreateShaderResourceView(Texture.Resource, &SrvDesc, Handle);
                }
            }
        }

        CHECKED_S(ComputeCommandAllocator->Reset());
        CHECKED_S(ComputeCommandList->Reset(ComputeCommandAllocator.Get(), ComputePipelineState.Get()));

        ComputeCommandList->SetComputeRootSignature(ComputeRootSignature.Get());

        // Iterate texture resources, fill command list and execute it
        {
            for (int i = 0; i < TexturesToProcess.size(); i++)
            {
                const TextureToProcess& Texture = TexturesToProcess[i];

                for (unsigned mipIndex = 0; mipIndex < TexturesToProcess[i].MipMapCount - 1; mipIndex++)
                {
                    CD3DX12_GPU_DESCRIPTOR_HANDLE Handle {
                        ComputeGpuSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
                        i * 2,
                        DescriptorOffset
                    };
                    ComputeCommandList->SetComputeRootDescriptorTable(0, Handle);

                    uint32_t RootConstants[] = {
                        Texture.MipZeroSize,
                        mipIndex,
                        Texture.bShouldNormalize ? 1u : 0u,
                    };
                    ComputeCommandList->SetComputeRoot32BitConstants(1, std::size(RootConstants), RootConstants, 0);

                    constexpr static unsigned THREAD_GROUP_SIZE = 32;
                    constexpr static unsigned PROCESS_BLOCK_SIZE = 2;

                    // we iterate using 2x2 blocks, so reduce dispatch size here and in shader, thread each index as a step of 2
                    unsigned DispatchSize = std::max(1u, (Texture.MipZeroSize % THREAD_GROUP_SIZE) / PROCESS_BLOCK_SIZE);
                    ComputeCommandList->Dispatch(DispatchSize, DispatchSize, 1);
                }
            }
        }

        CHECKED_S(ComputeCommandList->Close());

        ID3D12CommandList* CommandLists[] = {ComputeCommandList.Get()};
        RenderApi.GetComputeQueue()
            ->ExecuteCommandLists(1, CommandLists);

        return true;
    }

}
