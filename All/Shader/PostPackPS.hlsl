Texture2D gInput :register(t0);
SamplerState LinearRepeat :register(s0);
struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET
{
	float c = gInput.Sample(LinearRepeat, pin.Tex).r;

	uint int_color = asuint(c);

	float4 color;
	color.r = (float)(int_color >> 21) / 2047.0f;
	color.g = (float)((int_color >> 10) & 0x7ff) / 2047.0f;
	color.b = (float)(int_color & 0x0003ff) / 1023.0f;
	color.a = 1;

	return color;
}