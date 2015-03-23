Texture2D gInput :register(t0);
SamplerState LinearRepeat :register(s0);
struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET
{
	return gInput.Sample(LinearRepeat, pin.Tex);
}