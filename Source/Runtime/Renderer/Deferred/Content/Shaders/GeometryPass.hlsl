
struct VSInput
{
    float3 Position : POSITION;
    float2 Uv : UV;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct PSInput
{
    float4 NDC : SV_POSITION;
    float3 WorldPosition : WORLD_POSITION;
    float2 Uv: UV;
    float3 WorldNormal : NORMAL;
    float3x3 TBNMatrix : TBN_MATRIX;
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

struct ConstBuff_Frame
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;

    bool bHasAmbientLight;
    float3 AmbientColor;
    float AmbientIntensity;

    uint bHasDirectionalLight;
    float3 DirectionalColor;
    float3 DirectionalDir;
    float DirectionalIntensity;

    int2 ViewportSize;
    float3 CameraPosition;
};

struct ConstBuff_TexturedMesh
{
    float4x4 NormalMatrix;
    float4x4 ModelMatrix;

    bool bHasNormalMap;
};

// 0 - diffuse
// 1 - metallic
// 2 - roughness
// 3 - normal
// 4 - emissive
Texture2D TexturedMeshTextures[5] : register(t0);

ConstantBuffer<ConstBuff_Frame> FrameData : register(b0);
ConstantBuffer<ConstBuff_TexturedMesh> TexturedMeshData : register(b1);

PSInput VS_Main(VSInput Input)
{
    PSInput Output = (PSInput)0;

    Output.NDC = float4(Input.Position, 1.0f);

    return Output;
}

PSOutput PS_Main(PSInput Input)
{
    PSOutput Output = (PSOutput)0;

    return Output;
}