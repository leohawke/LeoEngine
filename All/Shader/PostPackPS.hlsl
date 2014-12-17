Texture2D gInput :register(t0);
SamplerState LinearRepeat :register(s0);
struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float color = gInput.Sample(LinearRepeat, pin.Tex).r;


	if(color < 1.f)
		return float4(sin(color),cos(color), 0.f, 1.f);

	return float4(color,0.f,0.f,1.f);
}