// 0 - diffuse
// 1 - world position
// 2 - world normal
// 3 - metallic
// 4 - roughness
// 5 - emissive
Texture2D GBufferTextures[5] : register(t0);

float3 GetGBufferDiffuseColor(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[0].Sample(Sampler, Uv).rgb;
}

float3 GetGBufferWorldPosition(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[1].Sample(Sampler, Uv).rgb;
}

float3 GetGBufferNormal(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[2].Sample(Sampler, Uv).rgb;
}

float GetGBufferMetallic(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[3].Sample(Sampler, Uv).r;
}

float GetGBufferRoughness(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[4].Sample(Sampler, Uv).r;
}

float4 GetGBufferEmissive(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[5].Sample(Sampler, Uv).rgba;
}
