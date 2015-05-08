cbuffer PointLight
{
	float4 diffuse;
	float4 position;//w : range
	float4 att;//ignore w;
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

	float pz = NormalDepth.w;
	float3 PosV = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;

	float3 lightVec = position.xyz - PosV;
	float d = max(length(lightVec),1.f);
	lightVec /= d;
	float lambert = dot(lightVec, NormalDepth.xyz);
	if (d > position.w)
		lambert = 0.f;

	float kd = 0.f;

	[flatten]
	if (lambert > 0.f)
	{
		float ka = 1.0f;// dot(att.xyz, float3(1.0f, d, d*d));
		kd = lambert*ka;
	}

	kd = max(0.1f, kd);
	
	return kd*diffuse *float4(Diffuse,1.f)*(Ambient);
}