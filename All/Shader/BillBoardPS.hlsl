SamplerState LinearRepeat :register(s0);
Texture2D TexDiffuse :register(t0);

struct PSInput
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float2 Tex : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
	float4 texColor = TexDiffuse.Sample(LinearRepeat, input.Tex);
	clip(texColor.a - 0.1f);

	return texColor;
}