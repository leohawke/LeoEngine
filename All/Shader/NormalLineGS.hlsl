cbuffer cbChangeEveryCamera : register(b0)
{
	matrix gViewProj;
};


struct VertexOut
{
	float3 PosW     : POSITION;
	float3 NormalW  : NORMAL;
};

struct GSOutput
{
	float4 PosH : SV_POSITION;
};

[maxvertexcount(2)]
void main(
	point VertexOut input[1],
	inout LineStream< GSOutput > output
)
{
	GSOutput element;
	element.PosH = mul(float4(input[0].PosW, 1.0f), gViewProj);
	output.Append(element);
	float4 end = float4(input[0].PosW + input[0].NormalW, 1.0f);
	element.PosH = mul(end, gViewProj);
	output.Append(element);
}