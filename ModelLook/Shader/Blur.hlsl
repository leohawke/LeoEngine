cbuffer BlurParam :register(b0) {
	float4 src_tex_size;
	float4 pack_color_weight[2];
	float4 pack_tex_coord_offset[2];
};


void BlurXVS(float4 pos : POSITION,
	float2 Tex0 : TEXCOORD0,
	out float4 oTex0 : TEXCOORD0,
	out float4 oTex1 : TEXCOORD1,
	out float4 oTex2 : TEXCOORD2,
	out float4 oTex3 : TEXCOORD3,
	out float2 oOriTex : TEXCOORD4,
	out float4 oPos : SV_Position)
{
	oPos = pos;

	float4 tex[4];
	
	float tex_coord_offset[8] = pack_tex_coord_offset;

	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		tex[i] = Tex0.xyxy + float4(tex_coord_offset[i * 2 + 0], 0, tex_coord_offset[i * 2 + 1], 0);
	}
	oTex0 = tex[0];
	oTex1 = tex[1];
	oTex2 = tex[2];
	oTex3 = tex[3];
	oOriTex = Tex0;
}

void BlurYVS(float4 pos : POSITION,
	float2 Tex0 : TEXCOORD0,
	out float4 oTex0 : TEXCOORD0,
	out float4 oTex1 : TEXCOORD1,
	out float4 oTex2 : TEXCOORD2,
	out float4 oTex3 : TEXCOORD3,
	out float2 oOriTex : TEXCOORD4,
	out float4 oPos : SV_Position)
{
	oPos = pos;

	float4 tex[4];

	float tex_coord_offset[8] = pack_tex_coord_offset;

	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		tex[i] = Tex0.xyxy + float4(0, tex_coord_offset[i * 2 + 0], 0, tex_coord_offset[i * 2 + 1]);
	}
	oTex0 = tex[0];
	oTex1 = tex[1];
	oTex2 = tex[2];
	oTex3 = tex[3];
	oOriTex = Tex0;
}

Texture2D src_tex:register(t0);
//filtering = min_mag_linear_mip_point
//address_u = clamp
//address_v = clamp
SamplerState src_sampler:register(s0);

float4 CalcBlur(float4 iTex0, float4 iTex1, float4 iTex2, float4 iTex3, float2 offset)
{
	float4 color = float4(0, 0, 0, 1);
	float4 tex[4] = { iTex0, iTex1, iTex2, iTex3 };

	float color_weight[8] = pack_color_weight;

	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		tex[i] += offset.xyxy;
		color.rgb += src_tex.Sample(src_sampler, tex[i].xy).rgb * color_weight[i * 2 + 0];
		color.rgb += src_tex.Sample(src_sampler, tex[i].zw).rgb * color_weight[i * 2 + 1];
	}

	return color;
}

float4 BlurXPS(float4 iTex0 : TEXCOORD0,
	float4 iTex1 : TEXCOORD1,
	float4 iTex2 : TEXCOORD2,
	float4 iTex3 : TEXCOORD3,
	float2 iOriTex : TEXCOORD4) : SV_Target0
{
	float2 offset = float2((floor(iOriTex.x * src_tex_size.x) + 0.5f) * src_tex_size.y - iOriTex.x, 0);
	return CalcBlur(iTex0, iTex1, iTex2, iTex3, offset);
}

float4 BlurYPS(float4 iTex0 : TEXCOORD0,
	float4 iTex1 : TEXCOORD1,
	float4 iTex2 : TEXCOORD2,
	float4 iTex3 : TEXCOORD3,
	float2 iOriTex : TEXCOORD4) : SV_Target0
{
	float2 offset = float2(0, (floor(iOriTex.y * src_tex_size.x) + 0.5f) * src_tex_size.y - iOriTex.y);
	return CalcBlur(iTex0, iTex1, iTex2, iTex3, offset);
}
