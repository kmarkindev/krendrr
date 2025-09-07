#pragma once

#include <wrl/client.h>
#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"
#include <d3dx12/d3dx12.h>
#include "Runtime/RenderApi/Core/RenderApi.h"

namespace krendrr::Runtime::Renderer::Core
{
    class PointLight
    {
    public:

        constexpr inline static unsigned SHADOW_MAP_SIZE = 1024;
        constexpr inline static DXGI_FORMAT CUBE_MAP_FORMAT = DXGI_FORMAT_R16G16B16A16_FLOAT;
        constexpr inline static DXGI_FORMAT DEPTH_STENCIL_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;

        [[nodiscard]] const glm::vec3& GetPosition() const;
        void SetPosition(const glm::vec3& NewPosition);

        [[nodiscard]] const glm::vec3& GetColor() const;
        void SetColor(const glm::vec3& NewColor);

        [[nodiscard]] float GetDistance() const;
        void SetDistance(float NewDistance);

        [[nodiscard]] float GetAttenuationLinear() const;
        void SetAttenuationLinear(float NewAttenuationLinear);
        [[nodiscard]] float GetAttenuationQuad() const;
        void SetAttenuationQuad(float NewAttenuationQuad);
        [[nodiscard]] float GetAttenuationConstant() const;

        [[nodiscard]] bool CastsShadows() const;
        void SetCastsShadows(bool NewCastsShadows);
        [[nodiscard]] float GetShadowFarDistance() const;
        bool HasShadowResources() const;
        bool CreateShadowCubeMapResource(const RenderApi::Core::RenderApi& RenderApi);
        D3D12_CPU_DESCRIPTOR_HANDLE GetShadowCubeMapSrvHandle() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetShadowCubeMapRtvHandle(int FaceIndex) const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetShadowCubeMapDsvHandle(int FaceIndex) const;
        void TransitionShadowCubeMapFromRenderTargetToRead(ID3D12GraphicsCommandList* CommandList);
        void TransitionShadowCubeMapFromReadToRenderTarget(ID3D12GraphicsCommandList* CommandList);

        bool UpdateConstantBuffer(const RenderApi::Core::RenderApi& RenderApi);

        D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGpuHandle() const;

    private:

        glm::vec3 Position {};

        glm::vec3 Color {1.f, 1.f, 1.f};

        bool bCastsShadows {true};

        float Distance {500.f};
        float AttenuationLinear {0.002f};
        float AttenuationQuad {0.0001f};
        float AttenuationConstant {1.f};

        struct alignas(256) ConstBuff_PointLight
        {
            glm::vec4 Position {};

            glm::vec4 DiffuseColor {};

            glm::vec3 SpecularColor {};
            float Distance {};

            float ShadowMapProjectionFarPlane {};
            float AttenuationLinear {};
            float AttenuationQuad {};
            float AttenuationConstant {};
        };

        Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBuffer {};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuSrvHeap {};

        Microsoft::WRL::ComPtr<ID3D12Resource> ShadowCubeMap {};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ShadowCubeMapSrvHeap {};
        unsigned RtvIncrementSize {};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ShadowCubeMapFacesRtvHeap {};
        Microsoft::WRL::ComPtr<ID3D12Resource> ShadowMapDepthStencil {};
        unsigned DsvIncrementSize {};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ShadowMapDepthDsvHeap {};
    };
}

