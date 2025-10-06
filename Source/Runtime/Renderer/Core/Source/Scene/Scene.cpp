#include "Runtime/Renderer/Core/Scene/Scene.h"
#include "Runtime/Renderer/Core/Scene/Lights/PointLight.h"
#include "Runtime/Renderer/Core/Scene/Objects/TexturedStaticMesh.h"

namespace krendrr::Runtime::Renderer::Core
{
    std::shared_ptr<TexturedStaticMesh> Scene::SpawnTexturedMesh()
    {
        auto NewMesh = std::make_shared<TexturedStaticMesh>();

        TexturedMeshes.push_back(NewMesh);

        // TODO: log spawn

        return NewMesh;
    }

    bool Scene::InsertTexturedMesh(const std::shared_ptr<TexturedStaticMesh>& NewMesh)
    {
        auto Iter = std::ranges::find(TexturedMeshes, NewMesh);
        if (Iter != TexturedMeshes.end())
        {
            // TODO: log mesh already inserted
            return false;
        }

        TexturedMeshes.push_back(NewMesh);

        // TODO: log success

        return true;
    }

    bool Scene::RemoveTexturedMesh(const std::shared_ptr<TexturedStaticMesh>& MeshToRemove)
    {
        auto Iter = std::ranges::find(TexturedMeshes, MeshToRemove);

        if (Iter == TexturedMeshes.end())
        {
            // TODO: log mesh not found
            return false;
        }

        // TODO: log success

        return true;
    }

    std::span<const std::shared_ptr<TexturedStaticMesh>> Scene::GetTexturedMeshes() const
    {
        return TexturedMeshes;
    }

    std::shared_ptr<PointLight> Scene::SpawnPointLight()
    {
        auto NewPointLight = std::make_shared<PointLight>();

        PointLights.push_back(NewPointLight);

        // TODO: log pawn

        return NewPointLight;
    }

    bool Scene::RemovePointLight(const std::shared_ptr<PointLight>& PointLightToRemove)
    {
        auto Iter = std::ranges::find(PointLights, PointLightToRemove);

        if (Iter == PointLights.end())
        {
            // TODO: log mesh not found
            return false;
        }

        // TODO: log success

        return true;
    }

    std::span<const std::shared_ptr<PointLight>> Scene::GetPointLights() const
    {
        return PointLights;
    }

    bool Scene::HasDirectionalLight() const
    {
        return bHasDirectionalLight;
    }

    const struct Scene::DirectionalLightData& Scene::GetDirectionalLightData() const
    {
        return DirectionalLightData;
    }

    void Scene::SetDirectionalLight(const struct DirectionalLightData& NewDirectionalLightData)
    {
        DirectionalLightData = NewDirectionalLightData;
    }

    void Scene::ToggleDirectionalLight(bool bNewHasDirectionalLight)
    {
        bHasDirectionalLight = bNewHasDirectionalLight;
    }

    void Scene::SetAmbientLightData(const struct AmbientLightData& NewAmbientLightData)
    {
        AmbientLightData = NewAmbientLightData;
    }

    void Scene::ToggleAmbientLight(bool bNewHasAmbientLight)
    {
        bHasAmbientLight = bNewHasAmbientLight;
    }

    bool Scene::HasAmbientLight() const
    {
        return bHasAmbientLight;
    }

    const struct Scene::AmbientLightData& Scene::GetAmbientLightData() const
    {
        return AmbientLightData;
    }

}
