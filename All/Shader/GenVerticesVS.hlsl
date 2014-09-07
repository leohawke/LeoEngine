static const float3 VoxelPoints[8] = { float3(0, 0, 0), float3(0, 0.03125f, 0), float3(0.03125f, 0.03125f, 0), float3(0.03125f, 0, 0),
		float3(0, 0, 0.03125f), float3(0, 0.03125f, 0.03125f), float3(0.03125f, 0.03125f, 0.03125f),
		float3(0.03125f, 0, 0.03125f) };


struct VertexIn
{
	float3 PosL : POSITION;
	uint Id : SV_InstanceID;
};

struct VertexOut
{
	float3 PosL : POSITION;
	float Density[8] : PSIZE;
	uint Case : POSITION1;
};

SamplerState DensitySample : register(s0);

Texture3D<float> densitySpace : register(t0);

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	uint id = vin.Id;
	float x = id % 32;
	float z = (id / 32) % 32;
	float y = id / 32 / 32;
	float3 xyz = float3(x, y, z) / 32.f;
	float3 texcoord;
	uint Case = 0;
	[unroll]
	for (int i = 0; i < 8; ++i)
	{
		texcoord = xyz+ VoxelPoints[i];
		//texcoord = float3(u, v, w);
		vout.Density[i] = densitySpace.SampleLevel(DensitySample, texcoord.xzy, 0);
		if (vout.Density[i] >= 0.f)
		{
			Case |= (1 << i);
		}
	}
	vout.Case = Case;
	vout.PosL = xyz;
	return vout;
}