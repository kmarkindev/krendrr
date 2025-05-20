struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

struct MatricesBuffer
{
    matrix MVP;
};

ConstantBuffer<MatricesBuffer> matBuf : register(b0);

Texture2D colorTex : register(t0);
SamplerState linearSamp : register(s0);

PSInput VSMain(float3 position : POS, float2 uv : UV)
{
    PSInput result;

    result.position = mul(matBuf.MVP, float4(position, 1.f)); //mul(matBuf.MVP, float4(position, 1.f));
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return colorTex.Sample(linearSamp, input.uv);
}