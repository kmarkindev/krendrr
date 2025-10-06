#pragma once

#include <memory>
#include <span>
#include <vector>

#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"

namespace krendrr::Runtime::Renderer::Core
{
    class PointLight;
}

namespace krendrr::Runtime::Renderer::Core
{
    class TexturedStaticMesh;

    class Scene
    {
    public:

        std::shared_ptr<TexturedStaticMesh> SpawnTexturedMesh();
        bool InsertTexturedMesh(const std::shared_ptr<TexturedStaticMesh>& NewMesh);
        bool RemoveTexturedMesh(const std::shared_ptr<TexturedStaticMesh>& MeshToRemove);
        [[nodiscard]] std::span<const std::shared_ptr<TexturedStaticMesh>> GetTexturedMeshes() const;

        std::shared_ptr<PointLight> SpawnPointLight();
        bool RemovePointLight(const std::shared_ptr<PointLight>& PointLightToRemove);
        [[nodiscard]] std::span<const std::shared_ptr<PointLight>> GetPointLights() const;

        struct DirectionalLightData
        {
            glm::vec3 Direction {-0.2f, -1.f, -0.3f};
            glm::vec3 Color {1.f, 1.f, 1.f};
            float Intensity {0.35f};
        };

        [[nodiscard]] bool HasDirectionalLight() const;
        [[nodiscard]] const DirectionalLightData& GetDirectionalLightData() const;
        void SetDirectionalLight(const DirectionalLightData& NewDirectionalLightData);
        void ToggleDirectionalLight(bool bNewHasDirectionalLight);

        struct AmbientLightData
        {
            glm::vec3 Color {1.f, 1.f, 1.f};
            float Intensity {0.0025f};
        };

        [[nodiscard]] bool HasAmbientLight() const;
        [[nodiscard]] const AmbientLightData& GetAmbientLightData() const;
        void SetAmbientLightData(const AmbientLightData& NewAmbientLightData);
        void ToggleAmbientLight(bool bNewHasAmbientLight);

    private:

        std::vector<std::shared_ptr<TexturedStaticMesh>> TexturedMeshes {};

        std::vector<std::shared_ptr<PointLight>> PointLights {};

        bool bHasDirectionalLight {false};
        DirectionalLightData DirectionalLightData {};

        bool bHasAmbientLight {false};
        AmbientLightData AmbientLightData {};

    };
}

