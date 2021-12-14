//
// Common
//
cbuffer ConstantValues : register(b0) {
    uint2   WindowPos;
    uint2   WindowSize;

    int     Step;
    int     NumCel;
    float   InvNumCel;

    float   Opacity;
    float   Scale;

    int3    Padding;
};

SamplerState Sampler : register(s0);

Texture2D CaptureTex : register(t0);

float2 GetProjected(in float2 UV)
{
    return float2(2.f * UV.x - 1.f, 1.f - 2.f * UV.y);
}

inline float CalcBrightness(const float3 color)
{
    return dot(float3(0.2126f, 0.7152f, 0.0722f), color);
}

inline float CalcSaturation(const float3 color)
{
    const float imax = max(max(color.r, color.g), color.b);
    const float imin = min(min(color.r, color.g), color.b);
    return imax > 0.f ? ((imax - imin)/imax) : (0.f);
    //float d = 1.f - abs(imax + imin - 1.f);
    //float s = (imax - imin) / max(d, 0.00001f);
    //return saturate(s);
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
void    ColorHistogramCS(
    in uint3 Id : SV_DispatchThreadID)
{
    const float3 c = CaptureTex[WindowPos + Step * Id.xy].rgb;
    const float b = CalcBrightness(c);

    InterlockedAdd(histogramBuffRW[255 * c.r].r, 1);
    InterlockedAdd(histogramBuffRW[255 * c.g].g, 1);
    InterlockedAdd(histogramBuffRW[255 * c.b].b, 1);
    InterlockedAdd(histogramBuffRW[255 * b].a, 1);
}

[numthreads(8, 8, 1)]
void    BrightnessHistogramCS(
    in uint3 Id : SV_DispatchThreadID)
{
    const float3 c = CaptureTex[WindowPos + Step * Id.xy].rgb;
    const float b = CalcBrightness(c);

    InterlockedAdd(histogramBuffRW[255 * b].a, 1);
}

[numthreads(8, 8, 1)]
void    SaturationHistogramCS(
    in uint3 Id : SV_DispatchThreadID)
{
    const float3 c = CaptureTex[WindowPos + Step * Id.xy].rgb;
    const float s = CalcSaturation(c);

    InterlockedAdd(histogramBuffRW[255 * s].a, 1);
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

float4 ColorPlanePS(
    float4 Position : SV_POSITION,
    float2 UV : TEXCOORD) : SV_Target0
{
    const float3 c = CaptureTex[WindowPos + WindowSize * UV].rgb;
    return float4(c, 1.f);
}

float4 BrightnessPlanePS(
    float4 Position : SV_POSITION,
    float2 UV : TEXCOORD) : SV_Target0
{
    const float3 c = CaptureTex[WindowPos + WindowSize * UV].rgb;
    const float b = CalcBrightness(c);
    return float4(b.xxx, 1.f);
}

float4 SaturationPlanePS(
    float4 Position : SV_POSITION,
    float2 UV : TEXCOORD) : SV_Target0
{
    const float3 c = CaptureTex[WindowPos + WindowSize * UV].rgb;
    const float s = pow(CalcSaturation(c), 1.f / 2.2f);
    return float4(s.xxx, 1.f);
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

void ColorGraphVS(
    in float2   UV : TEXCOORD,
    in uint     Index : HISTOINDEX,
    in uint     InstanceId : SV_InstanceID,
    out float4  OutPosition : SV_POSITION,
    out float2  OutUV : TEXCOORD,
    out float4  OutColor : COLOR)
{
    static const float4 Masks[4] = {
        float4(0.f, 1.f, 0.f, 0.f),
        float4(1.f, 0.f, 0.f, 0.f),
        float4(0.f, 0.f, 1.f, 0.f),
        float4(0.f, 0.f, 0.f, 1.f),
    };

    static const float3 Colors[4] = {
        float3(0.f, 1.f, 0.f),
        float3(1.f, 0.f, 0.f),
        float3(0.f, 0.f, 1.f),
        0.86f * float3(1.f, 1.f, 1.f),
    };

    const float histogram = dot(Masks[InstanceId], NormalizedHistogram(Index));
    const float y = Scale * UV.y * histogram;
    const float2 uv = float2(UV.x, 1.f - y);

    OutPosition = float4(GetProjected(uv), 0.f, 1.f);
    OutUV = uv;
    OutColor = float4(Colors[InstanceId], Opacity);
}

void MonoGraphVS(
    in float2   UV : TEXCOORD,
    in uint     Index : HISTOINDEX,
    out float4  OutPosition : SV_POSITION,
    out float2  OutUV : TEXCOORD,
    out float4  OutColor : COLOR)
{
    const float histogram = NormalizedHistogram(Index).a;
    const float y = Scale * UV.y * histogram;
    const float2 uv = float2(UV.x, 1.f - y);

    OutPosition = float4(GetProjected(uv), 0.f, 1.f);
    OutUV = uv;
    OutColor = float4(0.86f * float3(1.f, 1.f, 1.f), Opacity);
}

float4 FillGraphPS(
    in float4   Position : SV_POSITION,
    in float2   UV : TEXCOORD,
    in float4   Color : COLOR) : SV_TARGET
{
    return float4(Color.rgb, Color.a * lerp(0.1f, 1.f, UV.y));
}

float4 LineGraphPS(
    in float4   Position : SV_POSITION,
    in float2   UV : TEXCOORD,
    in float4   Color : COLOR) : SV_TARGET
{
    return float4(0.25f * Color.rgb, Color.a);
}