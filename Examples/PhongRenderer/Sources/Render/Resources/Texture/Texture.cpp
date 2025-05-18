#include "Texture.h"

#include <d3dx12/d3dx12_core.h>
#include <d3dx12/d3dx12_root_signature.h>

#include "Sources/Render/RenderDevice.h"
#include "Sources/Utils/HResultCheck.h"

void kRendrr::Texture::Initialize(const RenderDevice& RenderDevice, DXGI_FORMAT Format, glm::ivec2 Size, std::int8_t MipLevels)
{
    CheckInitialization(false);

    TextureFormat = Format;

    CD3DX12_HEAP_PROPERTIES HeapProps { D3D12_HEAP_TYPE_DEFAULT };
    CD3DX12_RESOURCE_DESC ResDesc = CD3DX12_RESOURCE_DESC::Tex2D(Format, Size.x, Size.y, MipLevels);

    RenderDevice.GetDevice()
        ->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &ResDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&TextureResource)
        ) >> HResultCheck {};

    MarkAsInitialized();
}

Microsoft::WRL::ComPtr<ID3D12Resource> kRendrr::Texture::GetTexture() const
{
    CheckInitialization();

    return TextureResource;
}

D3D12_SHADER_RESOURCE_VIEW_DESC kRendrr::Texture::GetSrvDesc() const
{
    return {
        .Format = TextureFormat,
        .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        .Texture2D = {
            .MostDetailedMip = 0,
            .MipLevels = static_cast<UINT>(-1),
        }
    };
}
