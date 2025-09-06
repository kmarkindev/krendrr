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