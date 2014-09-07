cbuffer PerBlock
{
	float4x4 gWorld;
	static const float3 yoffset = float3(0.0f, 1.0f / 32.0f, 0.0f);
};

struct VertexIn
{
	float3 PosL	:	POSITION;
	uint Id	: SV_InstanceID;
};

struct VertexOut
{
	float3 PosW	: POSITION;
	float4 PosH	: SV_POSITION;
	uint Id	: SV_InstanceID;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz + vin.Id*yoffset;
	vout.PosH = float4(vin.PosL.xzy, 1.0f);
	vout.Id = vin.Id;
	return vout;
}

struct GeoOut
{
	float3 PosW	: POSITION;
	float4 PosH	: SV_POSITION;
	uint Index	: SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> tristream)
{
	GeoOut gout;
	[unroll]
	for (int i = 0; i < 3; ++i)
	{
		gout.PosW = gin[i].PosW;
		gout.PosH = gin[i].PosH;
		gout.Index = gin[i].Id;
		tristream.Append(gout);
	}
	tristream.RestartStrip();
}

float4 PS(GeoOut pin) : SV_Target
{
	float4 density = float4(1.f,0.5f,0.25f,1.f);
	return density;
}

technique11 BuildDensity
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}