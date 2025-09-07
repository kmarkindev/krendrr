// 0 - diffuse
// 1 - metallic
// 2 - roughness
// 3 - normal
// 4 - emissive
Texture2D GBufferTextures[5] : register(t0);

float3 GetGBufferDiffuseColor(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[0].Sample(Sampler, Uv).rgb;
}

float GetGBufferMetallic(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[1].Sample(Sampler, Uv).r;
}

float GetGBufferRoughness(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[2].Sample(Sampler, Uv).r;
}

float3 GetGBufferNormal(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[3].Sample(Sampler, Uv).rgb;
}

float4 GetGBufferEmissive(SamplerState Sampler, float2 Uv)
{
    return GBufferTextures[4].Sample(Sampler, Uv).rgba;
}
