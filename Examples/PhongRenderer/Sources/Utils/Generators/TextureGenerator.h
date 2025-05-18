#pragma once

#include <vector>

namespace kRendrr
{
    /**
     * Generates 256x256 R8 G8 B8 A8 UINT texture
     */
    constexpr std::vector<std::uint8_t> GenerateCheckerTexture()
    {
        const std::uint8_t TextureWidth = 256;
        const std::uint8_t TextureHeight = 256;
        const std::uint8_t TexturePixelSize = 4;

        const std::uint8_t RowPitch = TextureWidth * TexturePixelSize;
        const std::uint8_t CellPitch = RowPitch >> 3;        // The width of a cell in the checkboard texture.
        const std::uint8_t CellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
        const std::uint8_t TextureSize = RowPitch * TextureHeight;

        std::vector<std::uint8_t> Data(TextureSize);

        for (std::uint8_t n = 0; n < TextureSize; n += TexturePixelSize)
        {
            std::uint8_t x = n % RowPitch;
            std::uint8_t y = n / RowPitch;
            std::uint8_t i = x / CellPitch;
            std::uint8_t j = y / CellHeight;

            if (i % 2 == j % 2)
            {
                Data[n] = 0x00;        // R
                Data[n + 1] = 0x00;    // G
                Data[n + 2] = 0x00;    // B
                Data[n + 3] = 0xff;    // A
            }
            else
            {
                Data[n] = 0xff;        // R
                Data[n + 1] = 0xff;    // G
                Data[n + 2] = 0xff;    // B
                Data[n + 3] = 0xff;    // A
            }
        }

        return Data;
    }
}
