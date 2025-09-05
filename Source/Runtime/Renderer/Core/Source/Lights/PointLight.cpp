#include "Runtime/Renderer/Core/Lights/PointLight.h"

#include "Runtime/RenderApi/Core/ConstBufferHelper.h"

namespace krendrr::Runtime::Renderer::Core
{
    const glm::vec3& PointLight::GetPosition() const
    {
        return Position;
    }

    void PointLight::SetPosition(const glm::vec3& NewPosition)
    {
        Position = NewPosition;
    }

    const glm::vec3& PointLight::GetColor() const
    {
        return Color;
    }

    void PointLight::SetColor(const glm::vec3& NewColor)
    {
        Color = NewColor;
    }

    float PointLight::GetDistance() const
    {
        return Distance;
    }

    void PointLight::SetDistance(float NewDistance)
    {
        Distance = NewDistance;
    }

    float PointLight::GetAttenuationLinear() const
    {
        return AttenuationLinear;
    }

    void PointLight::SetAttenuationLinear(float NewAttenuationLinear)
    {
        AttenuationLinear = NewAttenuationLinear;
    }

    float PointLight::GetAttenuationQuad() const
    {
        return AttenuationQuad;
    }

    void PointLight::SetAttenuationQuad(float NewAttenuationQuad)
    {
        AttenuationQuad = NewAttenuationQuad;
    }

    float PointLight::GetAttenuationConstant() const
    {
        return AttenuationConstant;
    }

    bool PointLight::CastsShadows() const
    {
        return bCastsShadows;
    }

    void PointLight::SetCastsShadows(bool NewCastsShadows)
    {
        bCastsShadows = NewCastsShadows;
    }

    bool PointLight::UpdateConstantBuffer(const RenderApi::Core::RenderApi& RenderApi)
    {
        // Create buffer if not created
        if (ConstantBuffer == nullptr)
        {
            if (!InitializeConstantBuffer<ConstBuff_PointLight>(RenderApi, ConstantBuffer, CpuSrvHeap, L"Point Light Constant Buffer"))
                return false;
        }

        // Update buffer

        ConstBuff_PointLight* Buffer {};
        CHECKED_S(ConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&Buffer)))

        *Buffer = {
            .ModelMatrix = glm::translate(glm::mat4(1.0f), Position),
            .Position = glm::vec4(Position, 0.f),
            .DiffuseColor = glm::vec4(Color, 0.f),
            .SpecularColor = Color,
            .Distance = Distance,
            .ShadowMapProjectionFarPlane = Distance + 1.f,
            .AttenuationLinear = AttenuationLinear,
            .AttenuationQuad = AttenuationQuad,
            .AttenuationConstant = AttenuationConstant
        };

        ConstantBuffer->Unmap(0, nullptr);

        return true;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE PointLight::GetConstantBufferHandle() const
    {
        if (CpuSrvHeap == nullptr)
            return {};

        return CpuSrvHeap->GetCPUDescriptorHandleForHeapStart();
    }
}
