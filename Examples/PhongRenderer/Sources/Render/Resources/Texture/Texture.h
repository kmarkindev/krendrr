#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "glm/vec2.hpp"
#include "Sources/Render/Resources/RenderResource.h"

namespace kRendrr
{
    class RenderDevice;

    class Texture : public RenderResource
    {
    public:

        void Initialize(const RenderDevice& RenderDevice, DXGI_FORMAT Format, glm::ivec2 Size, std::int8_t MipLevels);

        Microsoft::WRL::ComPtr<ID3D12Resource> GetTexture() const;

        D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc() const;

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> TextureResource {};
        DXGI_FORMAT TextureFormat {};
    };
}

