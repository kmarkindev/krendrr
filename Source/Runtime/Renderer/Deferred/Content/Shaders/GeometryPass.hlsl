
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
    float3 VertexWorldNormal : NORMAL;
    nointerpolation float3x3 TBNMatrix : TBN_MATRIX;
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

    float3 DirectionalColor;
    uint bHasDirectionalLight;
    float3 DirectionalDir;
    float DirectionalIntensity;

    float3 CameraPosition;
    int2 ViewportSize;
};

struct ConstBuff_PointLight
{
    float4x4 ModelMatrix;

    float3 Position;
    float3 DiffuseColor;
    float3 SpecularColor;

    float Distance;
    float ShadowMapProjectionFarPlane;

    float AttenuationLinear;
    float AttenuationQuad;
    float AttenuationConstant;
};

struct ConstBuff_TexturedMesh
{
    bool bHasNormalMap;

    float4x4 ModelMatrix;
    float3x3 NormalMatrix;
};

float3 CalculateWorldPosition(float4x4 ModelMatrix, float3 VertexPosition)
{
    float4 HomoWorldPos = mul(ModelMatrix, float4(VertexPosition, 1.0f));

    return HomoWorldPos.xyz / HomoWorldPos.w;
}

float4 CalculateNDC(float4x4 ModelMatrix, float4x4 ViewMatrix, float4x4 ProjectionMatrix, float3 VertexPosition)
{
    return mul(
        ProjectionMatrix,
        mul(
            ViewMatrix,
            mul(
                ModelMatrix,
                float4(VertexPosition, 1.0f)
            )
        )
    );
}

float3 CalculateNormal(float3 BaseNormal, float3x3 NormalMatrix)
{
    return normalize(
        mul(
            NormalMatrix,
            BaseNormal
        )
    );
}

float3 CalculateTangent(float3 BaseTangent, float3 TransformedNormal, float3x3 NormalMatrix)
{
    float3 Tangent = normalize(
        mul(
            NormalMatrix,
            BaseTangent
        )
    );

    // Gram-Schmidt process
    Tangent = normalize(
        Tangent - dot(Tangent, TransformedNormal) * TransformedNormal
    );

    return Tangent;
}

float3x3 CreateTBNMatrix(float3 Normal, float3 Tangent)
{
    float3 Bitangent = cross(Normal, Tangent);

    return float3x3(
        Tangent,
        Bitangent,
        Normal
    );
}

// 0 - diffuse
// 1 - metallic
// 2 - roughness
// 3 - normal
// 4 - emissive
Texture2D TexturedMeshTextures[5] : register(t0);

SamplerState DefaultSampler : register(s0);
SamplerState PointSampler : register(s1);

ConstantBuffer<ConstBuff_Frame> FrameData : register(b0);
ConstantBuffer<ConstBuff_TexturedMesh> TexturedMeshData : register(b1);

PSInput VS_Main(VSInput Input)
{
    PSInput Output = (PSInput)0;

    Output.NDC = CalculateNDC(TexturedMeshData.ModelMatrix, FrameData.ViewMatrix, FrameData.ProjectionMatrix, Input.Position);
    Output.Uv = Input.Uv;
    Output.WorldPosition = CalculateWorldPosition(TexturedMeshData.ModelMatrix, Input.Position);

    Output.VertexWorldNormal = CalculateNormal(Input.Normal, TexturedMeshData.NormalMatrix);
    float3 Tangent = CalculateTangent(Input.Tangent, Output.VertexWorldNormal, TexturedMeshData.NormalMatrix);
    Output.TBNMatrix = CreateTBNMatrix(Output.VertexWorldNormal, Tangent);

    return Output;
}

PSOutput PS_Main(PSInput Input)
{
    PSOutput Output = (PSOutput)0;

    Output.Diffuse = float4(TexturedMeshTextures[0].Sample(DefaultSampler, Input.Uv).rgb, 1.0f);
    Output.WorldPosition = float4(Input.WorldPosition, 1.0f);

    if(TexturedMeshData.bHasNormalMap)
    {
        float3 SampledNormal = TexturedMeshTextures[3].Sample(PointSampler, Input.Uv).rgb;
        SampledNormal = normalize(SampledNormal * 2.0f - 1.0f);

        Output.WorldNormal = float4(mul(Input.TBNMatrix, SampledNormal), 1.0f);


        // Convert from DirectX normal map to OpenGl normal map (left handed system to right handed system)
        //Output.WorldNormal.z = -Output.WorldNormal.z;
    }
    else
    {
        Output.WorldNormal = float4(Input.VertexWorldNormal, 1.0f);
    }

    Output.Metallic = float4(TexturedMeshTextures[1].Sample(DefaultSampler, Input.Uv).r, 0.0f, 0.0f, 1.0f);
    Output.Roughness = float4(TexturedMeshTextures[2].Sample(DefaultSampler, Input.Uv).r, 0.0f, 0.0f, 1.0f);

    Output.Emissive = TexturedMeshTextures[4].Sample(DefaultSampler, Input.Uv);
    Output.Emissive.a = length(Output.Emissive.rgb) == 0.0f ? 0.f : 1.f;

    return Output;
}