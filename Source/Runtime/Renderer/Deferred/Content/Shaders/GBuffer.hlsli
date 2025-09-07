Texture2D GBufferDiffuseTexture : register(t0);
Texture2D GBufferWorldPositionTexture : register(t1);
Texture2D GBufferWorldNormalTexture : register(t2);
Texture2D GBufferMetallicTexture : register(t3);
Texture2D GBufferRoughnessTexture : register(t4);
Texture2D GBufferEmissiveTexture : register(t5);

float3 GetGBufferDiffuseColor(SamplerState Sampler, float2 Uv)
{
    return GBufferDiffuseTexture.Sample(Sampler, Uv).rgb;
}

float3 GetGBufferWorldPosition(SamplerState Sampler, float2 Uv)
{
    return GBufferWorldPositionTexture.Sample(Sampler, Uv).rgb;
}

float3 GetGBufferNormal(SamplerState Sampler, float2 Uv)
{
    return GBufferWorldNormalTexture.Sample(Sampler, Uv).rgb;
}

float GetGBufferMetallic(SamplerState Sampler, float2 Uv)
{
    return GBufferMetallicTexture.Sample(Sampler, Uv).r;
}

float GetGBufferRoughness(SamplerState Sampler, float2 Uv)
{
    return GBufferRoughnessTexture.Sample(Sampler, Uv).r;
}

float4 GetGBufferEmissive(SamplerState Sampler, float2 Uv)
{
    return GBufferEmissiveTexture.Sample(Sampler, Uv).rgba;
}
