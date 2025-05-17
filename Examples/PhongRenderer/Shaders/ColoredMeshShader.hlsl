struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

PSInput VSMain(float3 position : POS, float2 uv : UV)
{
    PSInput result;

    result.position = float4(position, 1.f);
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(0.77, 0.24, 0.6, 1);
}