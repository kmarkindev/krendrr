#pragma once

#include <vector>
#include "Mesh.h"
#include "Texture.h"
#include "glm/vec3.hpp"

namespace krendrr::render
{
    class Model
    {
    public:

        struct TexturedMesh
        {
            Mesh Mesh {};
            Texture DiffuseTexture {};
            Texture SpecularTexture {};
            Texture NormalTexture {};

            glm::vec3 Color {};
        };

        struct LoadParams
        {
            bool bFlipUVs = false;
        };

        void Load(const std::string_view& ModelFileName, const LoadParams& Params = {});

        [[nodiscard]]
        std::span<const TexturedMesh> GetMeshes() const;

    private:

        std::vector<TexturedMesh> Meshes {};
    };
}
