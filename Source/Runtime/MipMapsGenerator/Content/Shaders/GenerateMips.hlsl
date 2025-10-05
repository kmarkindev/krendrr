struct ConstBuff_RootConstants
{
    uint CurrentMipSize;
    uint CurrentMipIndex;
    bool ShouldNormalize;
};

RWTexture2D<float4> CurrentMip : register(u0);
RWTexture2D<float4> NextMip : register(u1);

ConstantBuffer<ConstBuff_RootConstants> RootConstants : register(b0);

// Each shader invokation processes 2x2 blocks of mip level N, to generate one pixel for mip level N+1.
// 8x8 is optimal here, since it fits 32 and 64 warp sizes and covers most mip map sizes
[numthreads(8, 8, 1)]
void CS_Main(uint3 Id : SV_DispatchThreadID)
{
    uint xIndex = Id.x * 2;
    uint yIndex = Id.y * 2;

    // we process in blocks of 8x8, so in case current mip level has size less than 8x8 (like 4x4 and less),
    // cancel shader invokations that are not inside provided mip level
    if(xIndex + 1 > RootConstants.CurrentMipSize || yIndex + 1 > RootConstants.CurrentMipSize)
        return;

    float4 tlPixel = CurrentMip.Load(int2(xIndex, yIndex));
    float4 trPixel = CurrentMip.Load(int2(xIndex + 1, yIndex));
    float4 blPixel = CurrentMip.Load(int2(xIndex, yIndex + 1));
    float4 brPixel = CurrentMip.Load(int2(xIndex + 1, yIndex + 1));

    if(RootConstants.ShouldNormalize)
    {
        // used for normals (directions)

        // unpack directions (0..1 -> -1..1)
        tlPixel = normalize(tlPixel * 2 - 1);
        trPixel = normalize(trPixel * 2 - 1);
        blPixel = normalize(blPixel * 2 - 1);
        brPixel = normalize(brPixel * 2 - 1);

        // find average direction
        float4 avgDirection = normalize(tlPixel + trPixel + blPixel + brPixel);
        avgDirection.a = 1.0;

        // pack it into texture (-1..1 -> 0..1)
        NextMip[uint2(Id.x, Id.y)] = (avgDirection + 1) / 2;
    }
    else
    {
        // used for colors and masks
        NextMip[uint2(Id.x, Id.y)] = (tlPixel + trPixel + blPixel + brPixel) / 4;
    }
}