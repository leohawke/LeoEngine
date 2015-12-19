struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane :TEXCOORD0;//view_dir
	float2 Tex : TEXCOORD1;
};


float4 main(VertexOut pin) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}