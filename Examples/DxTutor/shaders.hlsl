struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct Transformation
{
    matrix mvp;
};

ConstantBuffer<Transformation> trans : register(b0);

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = mul(position, trans.mvp);
    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}