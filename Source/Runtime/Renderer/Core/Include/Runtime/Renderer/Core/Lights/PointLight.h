#pragma once

#include "glm/vec3.hpp"

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

    private:

        glm::vec3 Position {};

        glm::vec3 Color {1.f, 1.f, 1.f};

        float Distance {600.f};
        float AttenuationLinear {0.007f};
        float AttenuationQuad {0.0002f};
        float AttenuationConstant {1.f};
    };
}

