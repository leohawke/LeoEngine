Texture2D<float> gInput :register(t0);
SamplerState LinearRepeat :register(s0);
struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float color = gInput.Sample(LinearRepeat, pin.Tex).x;

	return float4(color,color,color,1.f);
}