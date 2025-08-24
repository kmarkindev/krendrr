#include "Runtime/Renderer/Core/Scene/Scene.h"

#include "Runtime/Renderer/Core/Lights/PointLight.h"
#include "Runtime/Renderer/Core/TexturedMesh/TexturedMesh.h"

namespace krendrr::Runtime::Renderer::Core
{
    std::shared_ptr<TexturedMesh> Scene::SpawnTexturedMesh()
    {
        auto NewMesh = std::make_shared<TexturedMesh>();

        TexturedMeshes.push_back(NewMesh);

        // TODO: log spawn

        return NewMesh;
    }

    bool Scene::InsertTexturedMesh(const std::shared_ptr<TexturedMesh>& NewMesh)
    {
        auto Iter = std::ranges::find(TexturedMeshes, NewMesh);
        if (Iter == TexturedMeshes.end())
        {
            // TODO: log mesh already inserted
            return false;
        }

        TexturedMeshes.push_back(NewMesh);

        // TODO: log success

        return true;
    }

    bool Scene::RemoveTexturedMesh(const std::shared_ptr<TexturedMesh>& MeshToRemove)
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

    std::span<const std::shared_ptr<TexturedMesh>> Scene::GetTexturedMeshes() const
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
