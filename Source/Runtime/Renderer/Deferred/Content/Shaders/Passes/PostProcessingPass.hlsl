#include <krendrr_runtime_renderer_deferred/Shaders/VSInput.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/FrameConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/Math/CommonScreenSpaceMath.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/GBuffer.hlsli>
#include <krendrr_runtime_renderer_Deferred/Shaders/StaticSamplers.hlsli>

Texture2D LightPassColorTexture : register(t6);
ConstantBuffer<ConstBuff_Frame> FrameData : register(b0);

float4 VS_Main(VSInput Input) : SV_POSITION
{
    return float4(Input.Position, 1.0f);
}

float4 PS_Main(float4 PixelPosition : SV_POSITION) : SV_TARGET0
{
    float2 Uv = CalculateScreenUv(FrameData.ViewportSize, PixelPosition);

    float3 LightPassColor = LoadTextureBySvPosition(LightPassColorTexture, PixelPosition).rgb;
    float4 EmissiveColor = GetGBufferEmissive(PixelPosition);

    float3 LdrColor;

    if(EmissiveColor.a > 0.f)
    {
        LdrColor = EmissiveColor.rgb;
    }
    else
    {
        float3 HdrColor = LightPassColor;

        // Reinhard tone mapping
        LdrColor = HdrColor / (HdrColor + 1.0f);
    }

    // Gamma Correction
    float Gamma = 2.2f;
    float3 GammaCorrectedColor = pow(abs(LdrColor), 1.0f / Gamma);

    return float4(GammaCorrectedColor, 1.0f);
}
