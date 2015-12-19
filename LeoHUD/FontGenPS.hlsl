struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane :TEXCOORD0;//view_dir
	float2 Tex : TEXCOORD1;
};


Texture2D alpha;
SamplerState s;

float4 main(VertexOut pin) : SV_TARGET
{
	return alpha.Sample(s,pin.Tex).rrrr;
}