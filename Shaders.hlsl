//
// Common
//
cbuffer ConstantValues : register(b0) {
    uint2   WindowPos;
    uint2   WindowSize;

    int     Step;
    int     NumCel;
    float   InvNumCel;

    float   GraphOpacity;
    float   GraphScale;

    int3    Padding;
};

SamplerState Sampler : register(s0);

Texture2D CaptureTex : register(t0);

float2 GetProjected(in float2 UV)
{
    return float2(2.f * UV.x - 1.f, 1.f - 2.f * UV.y);
}

//
// Compute Pass
//
RWStructuredBuffer<uint4> histogramBuffRW : register(u1);

[numthreads(1, 1, 1)]
void    ClearCS(uint Id : SV_DispatchThreadID)
{
    histogramBuffRW[Id] = uint4(0, 0, 0, 0);
}

[numthreads(8, 8, 1)]
void    CalcCS(
    in uint3 Id : SV_DispatchThreadID)
{
    const uint2 pixPos = WindowPos + Step * Id.xy;
    const float3 pix = CaptureTex[pixPos].rgb;
    const float lum = dot(float3(0.3f, 0.5f, 0.2f), pix);

    InterlockedAdd(histogramBuffRW[(uint)255 * pix.r].r, 1);
    InterlockedAdd(histogramBuffRW[(uint)255 * pix.g].g, 1);
    InterlockedAdd(histogramBuffRW[(uint)255 * pix.b].b, 1);
    InterlockedAdd(histogramBuffRW[(uint)255 * lum].a, 1);
}

//
// Plane Pass 
//
void PlaneVS(
    in float2 UV : TEXCOORD,
    out float4 OutPosition : SV_POSITION,
    out float2 OutUV : TEXCOORD)
{
    OutPosition = float4(GetProjected(UV), 0.f, 1.f);
    OutUV = UV;
}

float4 PlanePS(
    float4 Position : SV_POSITION,
    float2 UV : TEXCOORD) : SV_Target0
{
    return CaptureTex.Sample(Sampler, UV);
    //return CaptureTex[WindowPos + WindowSize * UV];
}

//
// Graph Pass
//
StructuredBuffer<uint4> histogramBuff : register(t1);

float4 NormalizedHistogram(int index)
{
    float4 values = histogramBuff[index];
    return (InvNumCel * values);
}

void GraphVS(
    in float2   UV : TEXCOORD,
    in uint     Index : HISTOINDEX,
    in uint     InstanceId : SV_InstanceID,
    out float4  OutPosition : SV_POSITION,
    out float2  OutUV : TEXCOORD,
    out float4  OutColor : COLOR)
{
    static const float3 Colors[4] = {
        float3(0.f, 1.f, 0.f),
        float3(1.f, 0.f, 0.f),
        float3(0.f, 0.f, 1.f),
        float3(1.f, 1.f, 1.f)
    };

    const float histogram = dot(Colors[InstanceId], NormalizedHistogram(Index));
    const float y = UV.y * histogram;
    const float2 uv = float2(UV.x, 1.f - GraphScale * y);

    OutPosition = float4(GetProjected(uv), 0.f, 1.f);
    OutUV = uv;
    OutColor = float4(Colors[InstanceId], histogram);
}

float4 GraphPS(
    in float4   Position : SV_POSITION,
    in float2   UV : TEXCOORD,
    in float4   Color : COLOR) : SV_TARGET
{
    return float4(Color.rgb, lerp(0.1f, GraphOpacity, UV.y));
}
