SamplerState LinearRepeat :register(s0);
TextureCube gCubeMap :register(t0);

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 Tex : TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET
{
	return gCubeMap.Sample(LinearRepeat, pin.Tex);
}