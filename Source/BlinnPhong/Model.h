#pragma once

#include <memory>
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

            std::shared_ptr<Texture> BaseColorTexture {};
            std::shared_ptr<Texture> MetallicTexture {};
            std::shared_ptr<Texture> RoughnessTexture {};
            std::shared_ptr<Texture> NormalTexture {};

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
