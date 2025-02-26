struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : COLOR;
};

struct Transformation
{
    matrix mvp;
};

ConstantBuffer<Transformation> trans : register(b0);

PSInput VSMain(float4 position : POSITION, float2 uv : COLOR)
{
    PSInput result;

    result.position = mul(position, trans.mvp);
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(input.uv.x, input.uv.y, 0, 1);
}