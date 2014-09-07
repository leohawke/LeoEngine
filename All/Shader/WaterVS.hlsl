//注释用作以后的sexp分析,自动生成代码

cbuffer cbChangerEveryFrame : register(b0)//(share)
{
	matrix WorldViewProj;
}

struct VertexIn//(in)
{
	float3 PosL : POSITION;//(offset 32)
};

struct VertexOut//(out)
{
	float4 PosH : SV_POSITION;
};


VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.f), WorldViewProj);
	return vout;
}