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