#pragma once
#include <string_view>
#include <glad/gl.h>

namespace krendrr::render
{

class Texture
{
public:

    Texture();
    ~Texture();

    struct TextureLoadParams
    {
        GLint TextureWrapS = GL_REPEAT;
        GLint TextureWrapT = GL_REPEAT;
        GLint TextureMinFilter = GL_LINEAR_MIPMAP_LINEAR;
        GLint TextureMagFilter = GL_NEAREST;
        GLint ApiFormat = GL_RGBA8;
        GLint MipMapsCount = 0;
        bool bFlipTexture = false;
    };

    void Load(const std::string_view& TextureFileName, const TextureLoadParams& Params = {});

    void ActivateTexture(std::uint32_t TextureUnit) const;

private:

    GLuint TextureId;

};

}
