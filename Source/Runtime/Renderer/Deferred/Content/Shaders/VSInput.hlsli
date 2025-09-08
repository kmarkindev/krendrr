#ifndef __INCLUDE_VS_INPUT__
#define __INCLUDE_VS_INPUT__

struct VSInput
{
    float3 Position : POSITION;
    float2 Uv : UV;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

#endif