#include "Runtime/Renderer/Core/TexturedMesh/Texture.h"
#include <algorithm>
#include <stb_image.h>
#include <stdexcept>
#include <cmath>

namespace krendrr::Runtime::Renderer::Core
{
Texture::Texture(Texture&& Other) noexcept
{
    MoveFrom(Other);
}

Texture& Texture::operator=(Texture&& Other) noexcept
{
    MoveFrom(Other);
    return *this;
}

void Texture::MoveFrom(Texture& Other) noexcept
{
    TextureId = std::exchange(Other.TextureId, 0);
}

Texture::~Texture()
{
    if(TextureId > 0)
        glDeleteTextures(1, &TextureId);
}

bool Texture::IsLoaded() const
{
    return TextureId > 0;
}

bool Texture::Load(const std::string_view& TextureFileName, const TextureLoadParams& Params)
{
    if(IsLoaded())
    {
        // TODO: log error already loaded
        return false;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &TextureId);

    stbi_set_flip_vertically_on_load(Params.bFlipTexture);

    int Width {};
    int Height {};
    int Channels {};
    unsigned char* Data = stbi_load(TextureFileName.data(), &Width, &Height, &Channels, 0);

    if(!Data)
    {
        // TODO: log error "Failed to load texture. File is invalid."
        return false;
    }

    // Note: stbi returns channels only as 8-bit components, so make sure we use 8 bit per channel when specifying texture format
    GLint Format {};
    switch (Channels)
    {
        case 1:
            Format = GL_RED;
        break;
        case 2:
            Format = GL_RG;
        break;
        case 3:
            Format = GL_RGB;
        break;
        case 4:
            Format = GL_RGBA;
        break;
        default:
            // TODO: log error "Can't load texture, it has unsupported number of channels: " + std::to_string(Channels)
            return false;
    }

    GLint Levels = Params.MipMapsCount;
    if(Levels <= 0)
    {
        // calculate how many mip maps we need to generate for the full chain
        Levels = static_cast<GLint>(std::floor(std::log2(Width))) + 1;
    }

    glTextureStorage2D(TextureId, Levels, Params.ApiFormat, Width, Height);
    glTextureSubImage2D(TextureId, 0, 0, 0, Width, Height, Format, GL_UNSIGNED_BYTE, Data);
    glGenerateTextureMipmap(TextureId);

    glTextureParameteri(TextureId, GL_TEXTURE_WRAP_S, Params.TextureWrapS);
    glTextureParameteri(TextureId, GL_TEXTURE_WRAP_T, Params.TextureWrapT);
    glTextureParameteri(TextureId, GL_TEXTURE_MIN_FILTER, Params.TextureMinFilter);
    glTextureParameteri(TextureId, GL_TEXTURE_MAG_FILTER, Params.TextureMagFilter);

    stbi_image_free(Data);

    return true;
}

bool Texture::ActivateTexture(std::uint32_t TextureUnit) const
{
    if(!CheckLoaded())
        return false;

    std::uint32_t ClampedTextureUnit = std::clamp(TextureUnit, 0u, 15u);
    if (ClampedTextureUnit != TextureUnit)
    {
        // TODO: log warning
    }

    glBindTextureUnit(ClampedTextureUnit, TextureId);

    return true;
}

bool Texture::CheckLoaded() const
{
    if (!IsLoaded())
    {
        // TODO: log error "Trying to use unloaded texture"
        return false;
    }

    return true;
}

}
