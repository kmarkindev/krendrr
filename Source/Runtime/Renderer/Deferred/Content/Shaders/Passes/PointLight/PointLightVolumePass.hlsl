#include <krendrr_runtime_renderer_deferred/Shaders/VSInput.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/FrameConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/Math/CommonScreenSpaceMath.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/GBuffer.hlsli>
#include <krendrr_runtime_renderer_Deferred/Shaders/StaticSamplers.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/PointLightConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/Math/CommonVertexShaderMath.hlsli>

ConstantBuffer<ConstBuff_Frame> FrameData : register(b0);
ConstantBuffer<ConstBuff_PointLight> PointLightData : register(b1);

float4 VS_Main(VSInput Input) : SV_POSITION
{
    return CalculateNDC(PointLightData.ModelMatrix, FrameData.ViewMatrix, FrameData.ProjectionMatrix, Input.Position);
}

float4 PS_Main(float4 SvPosition : SV_POSITION) : SV_TARGET0
{
    return float4(0.5f, 0.5f, 0.5f, 1.0f);
}
