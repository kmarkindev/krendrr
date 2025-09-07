#ifndef __INCLUDE_GBUFFER__
#define __INCLUDE_GBUFFER__

#include <krendrr_runtime_renderer_deferred/Shaders/Math/CommonScreenSpaceMath.hlsli>

Texture2D GBufferDiffuseTexture : register(t0);
Texture2D GBufferWorldPositionTexture : register(t1);
Texture2D GBufferWorldNormalTexture : register(t2);
Texture2D GBufferMetallicTexture : register(t3);
Texture2D GBufferRoughnessTexture : register(t4);
Texture2D GBufferEmissiveTexture : register(t5);

float3 GetGBufferDiffuseColor(float4 SvPosition)
{
    return LoadTextureBySvPosition(GBufferDiffuseTexture, SvPosition).rgb;
}

float3 GetGBufferWorldPosition(float4 SvPosition)
{
    return LoadTextureBySvPosition(GBufferWorldPositionTexture, SvPosition).rgb;
}

float3 GetGBufferNormal(float4 SvPosition)
{
    return LoadTextureBySvPosition(GBufferWorldNormalTexture, SvPosition).rgb;
}

float GetGBufferMetallic(float4 SvPosition)
{
    return LoadTextureBySvPosition(GBufferMetallicTexture, SvPosition).r;
}

float GetGBufferRoughness(float4 SvPosition)
{
    return LoadTextureBySvPosition(GBufferRoughnessTexture, SvPosition).r;
}

float4 GetGBufferEmissive(float4 SvPosition)
{
    return LoadTextureBySvPosition(GBufferEmissiveTexture, SvPosition).rgba;
}

#endif