#include <krendrr_runtime_renderer_deferred/Shaders/VSInput.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/FrameConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/Math/CommonScreenSpaceMath.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/GBuffer.hlsli>
#include <krendrr_runtime_renderer_Deferred/Shaders/StaticSamplers.hlsli>

ConstantBuffer<ConstBuff_Frame> FrameData : register(b0);

float4 VS_Main(VSInput Input) : SV_POSITION
{
    return float4(Input.Position, 1.0f);
}

float4 PS_Main(float4 PixelPosition : SV_POSITION) : SV_TARGET0
{
    float2 Uv = CalculateScreenUv(FrameData.ViewportSize, PixelPosition);

    float3 DiffuseColor = GetGBufferDiffuseColor(DefaultSampler, Uv);
    float Metallic = GetGBufferMetallic(DefaultSampler, Uv);
    float Roughness = GetGBufferRoughness(DefaultSampler, Uv);
    float3 WorldNormal = GetGBufferNormal(DefaultSampler, Uv);

    return float4(DiffuseColor / 2.0f, 1.0f);
}
