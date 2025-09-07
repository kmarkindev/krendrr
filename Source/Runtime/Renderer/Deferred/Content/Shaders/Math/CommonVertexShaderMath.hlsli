#ifndef __INCLUDE_COMMON_VERTEX_SHADER_MATH__
#define __INCLUDE_COMMON_VERTEX_SHADER_MATH__

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

    return transpose(float3x3(
        Tangent,
        Bitangent,
        Normal
    ));
}

#endif