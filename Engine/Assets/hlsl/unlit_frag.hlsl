#if !VULKAN
#define layout(a,b)  
#else
#define register(a) blank
#endif

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    float4 projCoord : TANGENT;
};
    
#include "ubo.h"

float linstep( float low, float high, float v )
{
    return saturate( (v - low) / (high - low) );
}

layout(set=0, binding=1) Texture2D<float4> tex : register(t0);
layout(set=0, binding=5) Texture2D<float4> _ShadowMap : register(t1);
layout(set=0, binding=2) SamplerState sLinear : register(s0);
//layout(set=0, binding=6) SamplerState sLinearClamp : register(s1);

float VSM( float depth, float4 projCoord )
{
    float2 moments = _ShadowMap.SampleLevel( sLinear, projCoord.xy / projCoord.w, 0 ).rg;

    float variance = max( moments.y - moments.x * moments.x, -0.001f );

    float delta = depth - moments.x;
    float p = smoothstep( depth - 0.02f, depth, moments.x );
    float pMax = linstep( minAmbient, 1.0f, variance / (variance + delta * delta) );

    return saturate( max( p, pMax ) );
}

float4 main( VSOutput vsOut ) : SV_Target
{
    float depth = vsOut.projCoord.z / vsOut.projCoord.w;
    float shadow = max( 0.2f, VSM( depth, vsOut.projCoord ) );
    return /*tint */ tex.Sample( sLinear, vsOut.uv ) * shadow;
}
