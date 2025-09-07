#ifndef __INCLUDE_CONST_BUFFER_POINT_LIGHT__
#define __INCLUDE_CONST_BUFFER_POINT_LIGHT__

struct ConstBuff_PointLight
{
    float3 Position;
    float3 DiffuseColor;
    float3 SpecularColor;

    float Distance;
    float ShadowMapProjectionFarPlane;

    float AttenuationLinear;
    float AttenuationQuad;
    float AttenuationConstant;
};

#endif