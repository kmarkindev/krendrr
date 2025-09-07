float2 CalculateScreenUv(float2 ScreenSize, float4 PixelPosition)
{
    return PixelPosition.xy / ScreenSize;
}