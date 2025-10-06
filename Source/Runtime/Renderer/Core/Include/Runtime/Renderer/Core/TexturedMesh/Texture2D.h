#pragma once

#include <string_view>
#include <wrl/client.h>
#include <d3dx12/d3dx12.h>
#include "glm/vec2.hpp"
#include "Runtime/RenderApi/Core/RenderApi.h"

namespace krendrr::Runtime::Renderer::Core
{
    class Texture2D
    {
    public:

        Texture2D() = default;
        Texture2D(const Texture2D& Other) = delete;
        Texture2D& operator=(const Texture2D& Other) = delete;
        Texture2D(Texture2D&& Other) noexcept = default;
        Texture2D& operator=(Texture2D&& Other) noexcept = default;
        ~Texture2D() = default;

        struct TextureLoadParams
        {
            std::size_t MipMapsCount = 0;
            bool bFlipTexture = false;

            static TextureLoadParams Default()
            {
                // https://bugs.llvm.org/show_bug.cgi?id=36684
                return {};
            }
        };

        /**
         * This only checks if internal resource objects were created.
         * It doesn't check if async load operations were completed.
         */
        [[nodiscard]] bool IsLoaded() const;

        struct TextureLoadOperation
        {
            bool bWasSuccessful {};
            Microsoft::WRL::ComPtr<ID3D12Resource> TextureUploadBuffer {};

            bool WasSuccessful() const
            {
                return bWasSuccessful;
            }
        };

        /**
         * Creates upload heaps and fills provided command list with copy operations.
         * Caller need to execute the command list and keep upload buffers alive while copy is not finished.
         */
        TextureLoadOperation Load(const RenderApi::Core::RenderApi& RenderApi, ID3D12GraphicsCommandList& CommandList,
            const std::string_view& TextureFileName, const TextureLoadParams& Params = TextureLoadParams::Default());

        D3D12_CPU_DESCRIPTOR_HANDLE GetTextureHandle() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetTextureGpuAddress() const;

        Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const;
        DXGI_FORMAT GetFormat() const;
        glm::uvec2 GetSize() const;
        unsigned GetMipsCount() const;

    private:

        bool CheckLoaded() const;

        DXGI_FORMAT Format {};
        glm::uvec2 Size {};
        unsigned MipsCount {};

        Microsoft::WRL::ComPtr<ID3D12Resource> TextureBuffer {};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuSrvHeap {};

    };
}
