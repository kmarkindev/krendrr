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

    std::vector<std::shared_ptr<Renderer::Core::TexturedMesh>> LoadModel(const std::string_view& ModelFileName, const LoadParams& Params);
}
