#pragma once
#include "glm/vec3.hpp"

namespace krendrr::DeferredShading
{
    struct PointLight
    {
        glm::vec3 Position {};

        glm::vec3 DiffuseColor {};
        glm::vec3 SpecularColor {};

        float Intensity {1.0f};

        float Distance {10.f};

        float AttenuationLinear {0.f};

        float AttenuationQuad {0.f};

        float AttenuationConstant {1.f};
    };
}
