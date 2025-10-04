#define NOMINMAX

#include "Runtime/MipMapsGenerator/Generator.h"
#include "Runtime/RenderApi/Core/ApiCallCheck.h"
#include <d3dcompiler.h>
#include <algorithm>
#include <cmath>

namespace krendrr::Runtime::MipMapsGenerator
{

    bool Generator::GenerateMipMaps(const RenderApi::Core::RenderApi& RenderApi, const std::span<TextureToProcess>& TexturesToProcess)
    {
        // Allocate everything if not allocated
        if (!ComputePipelineState)
        {
            CD3DX12_ROOT_PARAMETER RootParams[2] {};

            CD3DX12_DESCRIPTOR_RANGE Ranges[2] {};
            Ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            Ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

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

                    HRESULT VSCompileResult = D3DCompileFromFile(L"../Content/krendrr_runtime_mipmapsgenerator/Shaders/GenerateMips.hlsl",
                        nullptr, nullptr, "Main", "cs_5_1",
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
                .CS = CD3DX12_SHADER_BYTECODE {Shader.Get()},
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
        constexpr unsigned DescriptorsPerInvocation = 2;

        // Create descriptors
        {
            // Allocate enough space by figuring out how many shader invocation we need
            {
                unsigned InvocationsCount = 0;

                for (int i = 0; i < TexturesToProcess.size(); i++)
                {
                    const TextureToProcess& Texture = TexturesToProcess[i];
                    InvocationsCount += Texture.MipMapCount - 1; // -1 since to not include mip map 0
                }

                if (InvocationsCount == 0)
                {
                    // TODO: log error
                    return false;
                }

                D3D12_DESCRIPTOR_HEAP_DESC HeapDesc {
                    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                    .NumDescriptors = InvocationsCount * DescriptorsPerInvocation, // two UAV handles for each invocation
                    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                };

                CHECKED(
                    RenderApi.GetDevice()
                        ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&ComputeGpuSrvUavDescriptorHeap)),
                    "Could not create descriptor heap"
                )
            }

            CD3DX12_CPU_DESCRIPTOR_HANDLE Handle {
                ComputeGpuSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
            };

            // Then fill descriptors, texture by texture, mip level by mip level
            for (unsigned i = 0; i < TexturesToProcess.size(); i++)
            {
                const TextureToProcess& Texture = TexturesToProcess[i];

                if (TexturesToProcess[i].MipMapCount == 0)
                {
                    // TODO: log error "invalid input. specify count > 0 to generate mipmaps"
                    return false;
                }

                for (unsigned mipIndex = 0; mipIndex < TexturesToProcess[i].MipMapCount - 1; mipIndex++)
                {
                    D3D12_UNORDERED_ACCESS_VIEW_DESC ExistingMipLevelDesc = CD3DX12_UNORDERED_ACCESS_VIEW_DESC::Tex2D(Texture.Format, mipIndex);
                    RenderApi.GetDevice()
                        ->CreateUnorderedAccessView(Texture.Resource, nullptr, &ExistingMipLevelDesc, Handle);

                    Handle.Offset(1, DescriptorOffset);

                    D3D12_UNORDERED_ACCESS_VIEW_DESC NonExistingMipLevelDesc = CD3DX12_UNORDERED_ACCESS_VIEW_DESC::Tex2D(Texture.Format, mipIndex + 1);
                    RenderApi.GetDevice()
                        ->CreateUnorderedAccessView(Texture.Resource, nullptr, &NonExistingMipLevelDesc, Handle);

                    Handle.Offset(1, DescriptorOffset);
                }
            }
        }

        CHECKED_S(ComputeCommandAllocator->Reset());
        CHECKED_S(ComputeCommandList->Reset(ComputeCommandAllocator.Get(), ComputePipelineState.Get()));

        ComputeCommandList->SetComputeRootSignature(ComputeRootSignature.Get());
        ComputeCommandList->SetDescriptorHeaps(1, ComputeGpuSrvUavDescriptorHeap.GetAddressOf());

        CD3DX12_GPU_DESCRIPTOR_HANDLE Handle {
            ComputeGpuSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
        };

        // Iterate texture resources, fill command list and execute it
        for (int i = 0; i < TexturesToProcess.size(); i++)
        {
            const TextureToProcess& Texture = TexturesToProcess[i];

            auto InBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                Texture.Resource,
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS
            );

            ComputeCommandList->ResourceBarrier(1, &InBarrier);

            for (unsigned mipIndex = 0; mipIndex < TexturesToProcess[i].MipMapCount - 1; mipIndex++)
            {
                ComputeCommandList->SetComputeRootDescriptorTable(0, Handle);
                Handle.Offset(DescriptorsPerInvocation, DescriptorOffset);

                unsigned CurrentMipSize = Texture.MipZeroSize / static_cast<unsigned>(std::pow(2, mipIndex));

                uint32_t RootConstants[] = {
                    CurrentMipSize,
                    mipIndex,
                    Texture.bShouldNormalize ? 1u : 0u
                };
                ComputeCommandList->SetComputeRoot32BitConstants(1, std::size(RootConstants), RootConstants, 0);

                constexpr static unsigned THREAD_GROUP_SIZE = 8;
                constexpr static unsigned PROCESS_BLOCK_SIZE = 2;

                // we iterate using 2x2 blocks, so reduce dispatch size here and in shader, thread each index as a step of 2
                unsigned DispatchSize = std::max(1u, (CurrentMipSize / THREAD_GROUP_SIZE) / PROCESS_BLOCK_SIZE);
                ComputeCommandList->Dispatch(DispatchSize, DispatchSize, 1);

                // Each next CS invocation depends on data from previous invocation,
                // so add a UAV barrier to restrict reads/writes while previous CS invocation is not done
                auto UavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(Texture.Resource);
                ComputeCommandList->ResourceBarrier(1, &UavBarrier);
            }

            auto OutBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                Texture.Resource,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_COMMON
            );

            ComputeCommandList->ResourceBarrier(1, &OutBarrier);
        }

        CHECKED_S(ComputeCommandList->Close());

        ID3D12CommandList* CommandLists[] = {ComputeCommandList.Get()};
        RenderApi.GetComputeQueue()
            ->ExecuteCommandLists(1, CommandLists);

        return true;
    }

}
