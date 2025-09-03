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

        bool UpdateConstantBuffer(const RenderApi::Core::RenderApi& RenderApi);
        D3D12_CPU_DESCRIPTOR_HANDLE GetConstantBufferHandle() const;

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
            glm::mat4 ModelMatrix {};

            glm::vec3 Position {};
            glm::vec3 DiffuseColor {};
            glm::vec3 SpecularColor {};

            float Distance {};
            float ShadowMapProjectionFarPlane {};

            float AttenuationLinear {};
            float AttenuationQuad {};
            float AttenuationConstant {};
        };

        Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBuffer {};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CpuSrvHeap {};

    };
}

