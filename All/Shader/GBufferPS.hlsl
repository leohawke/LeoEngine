Texture2D gInput :register(t0);
SamplerState LinearRepeat :register(s0);
struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

float4 main(VertexOut pin) : SV_TARGET
{
	return float4(gInput.SampleLevel(LinearRepeat, pin.Tex,0).rgb,1.f);
}