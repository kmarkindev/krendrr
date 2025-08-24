#pragma once

#include <string_view>
#include <glad/gl.h>

namespace krendrr::Runtime::Renderer::Core
{
    class Texture
    {
    public:

        Texture() = default;
        Texture(const Texture& Other) = delete;
        Texture& operator=(const Texture& Other) = delete;
        Texture(Texture&& Other) noexcept;
        Texture& operator=(Texture&& Other) noexcept;
        ~Texture();

        void MoveFrom(Texture& Other) noexcept;

        struct TextureLoadParams
        {
            GLint TextureWrapS = GL_REPEAT;
            GLint TextureWrapT = GL_REPEAT;
            GLint TextureMinFilter = GL_LINEAR_MIPMAP_LINEAR;
            GLint TextureMagFilter = GL_LINEAR;
            GLint ApiFormat = GL_RGBA8;
            GLint MipMapsCount = 0;
            bool bFlipTexture = false;
        };

        [[nodiscard]] bool IsLoaded() const;

        bool Load(const std::string_view& TextureFileName, const TextureLoadParams& Params = {});

        bool ActivateTexture(std::uint32_t TextureUnit) const;

    private:

        bool CheckLoaded() const;

        GLuint TextureId{};

    };
}
