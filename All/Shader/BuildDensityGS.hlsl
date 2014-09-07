 struct VertexOut
{
	float3 PosW	: POSITION;
	float4 PosH	: SV_POSITION;
	uint Id	: SV_InstanceID;
};

struct GeoOut
{
	float3 PosW	: POSITION;
	float4 PosH	: SV_POSITION;
	uint Index	: SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void main(triangle VertexOut gin[3], inout TriangleStream<GeoOut> tristream)
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