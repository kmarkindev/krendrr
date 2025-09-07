#include <krendrr_runtime_renderer_deferred/Shaders/VSInput.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/FrameConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/PointLightConstBuffer.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/Math/CommonVertexShaderMath.hlsli>
#include <krendrr_runtime_renderer_deferred/Shaders/ConstBuffers/TexturedMeshConstBuffer.hlsli>

struct ConstBuff_MatrixData
{
    float4x4 FaceViewProjectionMatrix;
};

ConstantBuffer<ConstBuff_Frame> FrameData : register(b0);
ConstantBuffer<ConstBuff_PointLight> PointLightData : register(b1);
ConstantBuffer<ConstBuff_TexturedMesh> TexturedMeshData : register(b2);
ConstantBuffer<ConstBuff_MatrixData> MatrixData : register(b3);

struct VSOutput
{
    float4 NDC : SV_POSITION;
    float3 WorldPosition : WORLD_POSITION;
};

VSOutput VS_Main(VSInput Input)
{
    VSOutput Output = (VSOutput)0;
    Output.NDC = CalculateNDC(TexturedMeshData.ModelMatrix, MatrixData.FaceViewProjectionMatrix, Input.Position);
    Output.WorldPosition = CalculateWorldPosition(TexturedMeshData.ModelMatrix, Input.Position);

    return Output;
}

float4 PS_Main(VSOutput Input) : SV_TARGET0
{
    return length(Input.WorldPosition - PointLightData.Position) / PointLightData.ShadowMapProjectionFarPlane;
}