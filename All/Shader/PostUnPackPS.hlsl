Texture2D<uint> gInput :register(t0);

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
};

float4 main(VertexOut pin) : SV_TARGET
{
	uint2 tex_dim;
	gInput.GetDimensions(tex_dim.x, tex_dim.y);

	uint2 location = uint2((uint)(tex_dim.x * pin.Tex.x), (uint)(tex_dim.y * pin.Tex.y));
	uint int_color = gInput[location];
	float4 color;
	color.r = (float)(int_color >> 21) / 2047.0f;
	color.g = (float)((int_color >> 10) & 0x7ff) / 2047.0f;
	color.b = (float)(int_color & 0x0003ff) / 1023.0f;
	color.a = 1;

	return color;
}