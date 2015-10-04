void PostProcessVS(float4 pos : POSITION,
	float2 tex: TEXCOORD0,
	out float2 Tex : TEXCOORD0,
	out float4 PosH : SV_Position)
{
	PosH = pos;
	Tex = tex;
}