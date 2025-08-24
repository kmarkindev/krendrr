#pragma once

#include <string_view>
#include <vector>
#include "Runtime/Renderer/Core/TexturedMesh/TexturedMesh.h"

namespace krendrr::Runtime::ModelLoader
{
    struct LoadParams
    {
        bool bFlipUVs = false;
        std::string_view DiffuseTextureName {"diffuse"};
        std::string_view MetallicTextureName {"metallic"};
        std::string_view RoughnessTextureName {"roughness"};
        std::string_view NormalTextureName {"normal"};
    };

    struct LoadResult
    {
        bool bSuccess {};
        std::vector<std::shared_ptr<Renderer::Core::TexturedMesh>> TexturedMeshes {};

        LoadResult()
            : bSuccess(false), TexturedMeshes{}
        {
        }

        [[nodiscard]] bool HasLoadedAtLeastOne() const
        {
            return bSuccess && !TexturedMeshes.empty();
        }
    };

    LoadResult LoadModel(const std::string_view& ModelFileName, const LoadParams& Params = {});
}
