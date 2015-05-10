cbuffer DirectionalLight : register(c0)
{
	float3 direction;//w : range
	float pad;
	float3 diffuse;
	float3 specPow;
}

Texture2D normalTex :register(t0);//w:depth
Texture2D diffuseTex :register(t1);

#ifdef DEBUG
Texture2D<float4> ambientTex:register(t2);
#else
Texture2D<float> ambientTex:register(t2);
#endif

SamplerState LinearRepeat :register(s0);
SamplerState samNormalDepth : register(s1);

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane :TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float4 NormalDepth = normalTex.SampleLevel(samNormalDepth,pin.Tex,0);
	NormalDepth.xyz = NormalDepth.xyz*2.f - 1.f;
	float3 Diffuse = diffuseTex.Sample(LinearRepeat, pin.Tex).rgb;
	float Ambient = ambientTex.SampleLevel(samNormalDepth, pin.Tex, 0).r;

	float lambert = dot(direction, NormalDepth.xyz);

	float kd = max(0.1f, lambert);

	return float4(kd*diffuse *Diffuse*Ambient,1.f);
}

