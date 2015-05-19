cbuffer perLight :register(b0) {
	matrix WorldViewProj;
	matrix Proj;
}

struct LightVolumePoint {
	float3 PosL :POSITION;
}


struct VertexOut {
	float3 ViewDir :POSITION;
	float4 PosH: SV_POSITION;
	float3 Tex :TEXCOORD;
};

VertexOut main(LightVolumePoint vin)
{
	VertexOut vout;
	vout.ViewDir = mul(float4(vin.PosL, 1.f), WorldView).xyz;
	vout.PosH = mul(float4(vout.ViewDir, 1.f), Proj);

	vout.Tex = vout.PosH.xyw;

	vout.Tex.xy = vout.Tex.xy*0.5f + 0.5f;
	vout.Tex.y = 1.f - vout.Tex.y;
	return pos;
}