#ifndef __INCLUDE_CONST_BUFFER_TEXTURED_MESH__
#define __INCLUDE_CONST_BUFFER_TEXTURED_MESH__

struct ConstBuff_TexturedMesh
{
    bool bHasNormalMap;

    float4x4 ModelMatrix;
    float3x3 NormalMatrix;
};

#endif