cbuffer cbChangerEveryFrame : register(b0)//(share)
{
	matrix World;
}

struct VertexIn
{
	float3 PosL : POSITION;
	uint Id : SV_InstanceID;
};

struct VertexOut
{
	float3 PosW : POSITION;
	float4 PosH : SV_POSITION;
	uint Id: SV_InstanceID;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	float3 pos = vin.PosL/ 2.f + float3(0.5f, 0, 0.5f);//map(-1,1) => (0,1)
	vout.PosW = mul(
				float4( pos+vin.Id *float3(0.0f, 1.0f / 32.0f, 0.0f) , 1.0f), 
				World).xyz;
	vout.PosH = float4(vin.PosL.xzy, 1.0f);
	vout.Id = vin.Id;
	return vout;
}