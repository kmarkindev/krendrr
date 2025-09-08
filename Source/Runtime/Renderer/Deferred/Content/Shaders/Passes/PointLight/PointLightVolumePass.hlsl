#include <krendrr_runtime_renderer_deferred/Shaders/VSInput.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/FrameConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/Math/CommonScreenSpaceMath.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/GBuffer.hlsli>
#include <krendrr_runtime_renderer_Deferred/Shaders/StaticSamplers.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/PointLightConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/Math/CommonVertexShaderMath.hlsli>

ConstantBuffer<ConstBuff_Frame> FrameData : register(b0);
ConstantBuffer<ConstBuff_PointLight> PointLightData : register(b1);

TextureCube ShadowCubeMap : register(t6);

float4 VS_Main(VSInput Input) : SV_POSITION
{
    return CalculateNDC(PointLightData.ModelMatrix, FrameData.ViewMatrix, FrameData.ProjectionMatrix, Input.Position);
}

float4 PS_Main(float4 SvPosition : SV_POSITION) : SV_TARGET0
{
    float3 DiffuseColor = GetGBufferDiffuseColor(SvPosition);
    float3 WorldPosition = GetGBufferWorldPosition(SvPosition);
    float3 WorldNormal = GetGBufferNormal(SvPosition);
    float Metallic = GetGBufferMetallic(SvPosition);
    float Roughness = GetGBufferRoughness(SvPosition);

    float3 LightDirection = normalize(WorldPosition - PointLightData.Position);
    float PointLightDistance = length(PointLightData.Position - WorldPosition);

    float Attenuation = 1.0 / (
        PointLightData.AttenuationConstant + PointLightData.AttenuationLinear * PointLightDistance + PointLightData.AttenuationQuad * (PointLightDistance * PointLightDistance)
    );

    float3 LightColor = float3(0.f, 0.f, 0.f);

    // Diffuse
    {
        float DiffuseScale = max(0.0f, dot(-LightDirection, WorldNormal));

        LightColor += max(0.0f, PointLightData.DiffuseColor * DiffuseScale * Attenuation);
    }

    // Specular
    {
        // TODO:
    }

    // Shadow
    {
        float ShadowMapDepthDistance = ShadowCubeMap.Sample(DefaultSampler, LightDirection).r * PointLightData.ShadowMapProjectionFarPlane;
        float ShadowBias = max(0.05 * (1.0 - dot(WorldNormal, LightDirection)), 0.005);

        float ShadowValue = ShadowMapDepthDistance - ShadowBias <= PointLightDistance ? 1.f : 0.f;

        // TODO: add PFC

        LightColor *= 1.0f - ShadowValue;
    }

    return float4(DiffuseColor * LightColor, 1.f);
}
