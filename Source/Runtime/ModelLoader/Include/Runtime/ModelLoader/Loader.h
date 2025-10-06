#pragma once

#include <string_view>
#include <vector>
#include "Runtime/RenderApi/Core/RenderApi.h"
#include "Runtime/Renderer/Core/TexturedMesh/TexturedStaticMesh.h"
#include "taskflow/taskflow.hpp"

namespace krendrr::Runtime::ModelLoader
{
    struct LoadParams
    {
        bool bFlipUVs = false;

        /**
         * There are two options: Y - up OR Z - up.
         * We are using Y as up, so if model has Z as up, we need to flip normals and tangents.
         */
        bool bFlipNormals = false;

        /**
         * Convert between left <-> right handed system for mesh normals and tangents
         */
        bool bNegateNormalZ = false;

        std::string_view DiffuseTextureName {"diffuse"};
        std::string_view MetallicTextureName {"metallic"};
        std::string_view RoughnessTextureName {"roughness"};
        std::string_view NormalTextureName {"normal"};
        std::string_view EmissiveTextureName {"emissive"};
    };

    struct LoadResult
    {
        bool bSuccess {};
        std::vector<std::shared_ptr<Renderer::Core::TexturedStaticMesh>> TexturedMeshes {};

        [[nodiscard]] bool HasLoadedAtLeastOne() const
        {
            return bSuccess && !TexturedMeshes.empty();
        }
    };

    LoadResult LoadModel(const std::string_view& ModelFileName, const RenderApi::Core::RenderApi& RenderApi, tf::Executor& TfExecutor, const LoadParams& Params = {});
}
