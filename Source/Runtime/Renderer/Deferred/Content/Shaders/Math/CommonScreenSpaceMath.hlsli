#ifndef __INCLUDE_COMMON_SCREENSPACE_MATH__
#define __INCLUDE_COMMON_SCREENSPACE_MATH__

float4 LoadTextureBySvPosition(Texture2D Texture, float4 SvPosition)
{
    return Texture.Load(int3((int2)SvPosition.xy, 0)).rgba;
}

float2 CalculateScreenUv(float2 ScreenSize, float4 PixelPosition)
{
    return PixelPosition.xy / ScreenSize;
}

#endif