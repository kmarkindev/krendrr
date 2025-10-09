#include "Runtime/RenderApi/Core/Resources/Texture2D.h"
#include <cassert>
#include <stb_image.h>
#include <stdexcept>
#include <cmath>
#include "Runtime/RenderApi/Core/ApiCallCheck.h"

namespace krendrr::Runtime::RenderApi::Core
{

bool Texture2D::IsLoaded() const
{
    return TextureBuffer != nullptr;
}

Texture2D::TextureLoadOperation Texture2D::Load(const RenderApi& RenderApi, ID3D12GraphicsCommandList& CommandList,
    const std::string_view& TextureFileName, const TextureLoadParams& Params)
{
    if(IsLoaded())
    {
        // TODO: log error already loaded
        return {};
    }

    // Load texture and determine its format

    stbi_set_flip_vertically_on_load(Params.bFlipTexture);

    int Width {};
    int Height {};
    int Channels {};

    if (stbi_info(TextureFileName.data(), &Width, &Height, &Channels) != 1)
    {
        // TODO: log error
        return {};
    }

    // DX12 doesn't have RGB format, only RGBA
    int DesiredChannels = 0;
    if (Channels == 3)
    {
        DesiredChannels = 4;
    }

    unsigned char* Data = stbi_load(TextureFileName.data(), &Width, &Height, &Channels, DesiredChannels);

    if(!Data)
    {
        // TODO: log error "Failed to load texture. File is invalid."
        return {};
    }

    Size = glm::uvec2(Width, Height);

    // Note: stbi returns channels only as 8-bit components, so make sure we use 8 bit per channel when specifying texture format
    switch (Channels)
    {
        case 1:
            Format = DXGI_FORMAT_R8_UNORM;
            Channels = 1;
        break;
        case 2:
            Format = DXGI_FORMAT_R8G8_UNORM;
            Channels = 2;
        break;
        case 3:
            Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            Channels = 4;
        break;
        case 4:
            Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            Channels = 4;
        break;
        default:
            // TODO: log error "Can't load texture, it has unsupported number of channels: " + std::to_string(Channels)
            assert(false);
            return {};
    }

    MipsCount = Params.MipMapsCount;
    if(MipsCount == 0)
    {
        // calculate how many mip maps we need to generate for the full chain
        MipsCount = std::floor(std::log2(Width)) + 1;
    }

    // Mips are not supported for non-square textures, or when size is not power of 2
    if (Size.x != Size.y || std::sqrt(Size.x) != std::floor(std::sqrt(Size.x)))
        MipsCount = 1;

    // Set up upload buffer
    const std::size_t TextureBufferSize = Width * Height * Channels;
    Microsoft::WRL::ComPtr<ID3D12Resource> TextureUploadBuffer = RenderApi.CreateUploadBufferAndMap(
        std::span<const std::byte>{reinterpret_cast<std::byte*>(Data), TextureBufferSize},
        true
    );

    // Create texture buffer

    const CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(Format, Width, Height, 1, MipsCount, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    const CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    CHECKED(
        RenderApi.GetDevice()
            ->CreateCommittedResource(
                &HeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &ResourceDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&TextureBuffer)
            ),
        "Could not create texture buffer"
    )

    D3D12_SUBRESOURCE_DATA SubresData {
        .pData = Data,
        .RowPitch = Width * Channels,
        .SlicePitch = Width * Height * Channels
    };

    UpdateSubresources(&CommandList, TextureBuffer.Get(), TextureUploadBuffer.Get(), 0, 0, 1, &SubresData);

    stbi_image_free(Data);

    // Create CPU descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc {
        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        .NumDescriptors = 1,
        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    };

    CHECKED(
        RenderApi.GetDevice()
            ->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&CpuSrvHeap)),
        "Could not create descriptor heap"
    )

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2D(
        Format
    );

    RenderApi.GetDevice()
        ->CreateShaderResourceView(TextureBuffer.Get(), &SRVDesc, CpuSrvHeap->GetCPUDescriptorHandleForHeapStart());

    TextureBuffer->SetName(L"Texture Buffer");
    TextureUploadBuffer->SetName(L"Upload Texture Buffer");

    return {
        .bWasSuccessful = true,
        .TextureUploadBuffer = TextureUploadBuffer,
    };
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture2D::GetTextureHandle() const
{
    if (CpuSrvHeap == nullptr)
        return {};

    return CpuSrvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_VIRTUAL_ADDRESS Texture2D::GetTextureGpuAddress() const
{
    return TextureBuffer->GetGPUVirtualAddress();
}

Microsoft::WRL::ComPtr<ID3D12Resource> Texture2D::GetResource() const
{
    return TextureBuffer;
}

DXGI_FORMAT Texture2D::GetFormat() const
{
    return Format;
}

glm::uvec2 Texture2D::GetSize() const
{
    return Size;
}

unsigned Texture2D::GetMipsCount() const
{
    return MipsCount;
}

bool Texture2D::CheckLoaded() const
{
    if (!IsLoaded())
    {
        // TODO: log error "Trying to use unloaded texture"
        return false;
    }

    return true;
}

}
