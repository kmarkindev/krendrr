#include <krendrr_runtime_renderer_Deferred/Shaders/VSInput.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/FrameConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/TexturedMeshConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/Math/CommonVertexShaderMath.hlsli>
#include <krendrr_runtime_renderer_Deferred/Shaders/StaticSamplers.hlsli>

struct VSOutput
{
    float4 NDC : SV_POSITION;
    float3 WorldPosition : WORLD_POSITION;
    float2 Uv: UV;

    float3 Normal : NORMAL;
    float3x3 TBNMatrix : TBN;
};

struct PSOutput
{
    float4 Diffuse : SV_Target0;
    float4 WorldPosition : SV_Target1;
    float4 WorldNormal : SV_Target2;
    float4 Metallic : SV_Target3;
    float4 Roughness : SV_Target4;
    float4 Emissive : SV_Target5;
};

// 0 - diffuse
// 1 - metallic
// 2 - roughness
// 3 - normal
// 4 - emissive
Texture2D TexturedMeshTextures[5] : register(t0);

ConstantBuffer<ConstBuff_Frame> FrameData : register(b0);
ConstantBuffer<ConstBuff_TexturedMesh> TexturedMeshData : register(b1);

VSOutput VS_Main(VSInput Input)
{
    VSOutput Output = (VSOutput)0;

    Output.NDC = CalculateNDC(TexturedMeshData.ModelMatrix, FrameData.ViewMatrix, FrameData.ProjectionMatrix, Input.Position);
    Output.Uv = Input.Uv;
    Output.WorldPosition = CalculateWorldPosition(TexturedMeshData.ModelMatrix, Input.Position);

    Output.Normal = CalculateNormal(Input.Normal, TexturedMeshData.NormalMatrix);
    float3 Tangent = CalculateTangent(Input.Tangent, Output.Normal, TexturedMeshData.NormalMatrix);
    Output.TBNMatrix = CreateTBNMatrix(Output.Normal, Tangent);

    return Output;
}

PSOutput PS_Main(VSOutput Input)
{
    PSOutput Output = (PSOutput)0;

    Output.Diffuse = float4(TexturedMeshTextures[0].Sample(DefaultSampler, Input.Uv).rgb, 1.0f);

    Output.WorldPosition = float4(Input.WorldPosition, 1.0f);

    Output.Metallic = float4(TexturedMeshTextures[1].Sample(DefaultSampler, Input.Uv).r, 0.0f, 0.0f, 1.0f);
    Output.Roughness = float4(TexturedMeshTextures[2].Sample(DefaultSampler, Input.Uv).r, 0.0f, 0.0f, 1.0f);

    Output.Emissive = TexturedMeshTextures[4].Sample(DefaultSampler, Input.Uv);
    Output.Emissive.a = length(Output.Emissive.rgb) == 0.0f ? 0.f : 1.f;

    if(TexturedMeshData.bHasNormalMap)
    {
        float3 SampledNormal = TexturedMeshTextures[3].Sample(DefaultSampler, Input.Uv).rgb;
        SampledNormal = normalize(SampledNormal * 2.0f - 1.0f);

        Output.WorldNormal = float4(mul(Input.TBNMatrix, SampledNormal), 1.0f);

        // Convert from DirectX normal map to OpenGl normal map (left handed system to right handed system)
        //Output.WorldNormal.z = -Output.WorldNormal.z;
    }
    else
    {
        Output.WorldNormal = float4(Input.Normal, 1.0f);
    }

    return Output;
}