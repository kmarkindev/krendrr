struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct Transformation
{
    matrix mvp;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);
ConstantBuffer<Transformation> trans : register(b0);

PSInput VSMain(float3 position : POSITION, float2 uv : TEXCOORD)
{
    PSInput result;

    result.position = mul(float4(position, 1.0f), trans.mvp);
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return tex.Sample(samp, input.uv);
}