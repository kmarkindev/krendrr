#include "Runtime/Renderer/Core/Lights/PointLight.h"

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
}
