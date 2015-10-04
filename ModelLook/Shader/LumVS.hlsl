cbuffer params :register(c0) {
	float4 tex_coord_offset[2];
}

void LumVS(float4 pos : POSITION,
	float2 tex:TEXCOORD0,
	out float4 oTex0 : TEXCOORD0,
	out float4 oTex1 : TEXCOORD1,
	out float4 oPos : SV_Position)
{
	oPos = pos;
	oTex0 = tex.xyxy + tex_coord_offset[0];
	oTex1 = tex.xyxy + tex_coord_offset[1];
}