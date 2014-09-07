cbuffer cbChangeEveryCamera : register(b0)
{
	float3 gEyePos;
	matrix gViewProj;
};

struct GSInput
{
	float3 CenterW : POSITION;
	float2 SizeW : SIZE;
};

struct GSOutput
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float2 Tex : TEXCOORD;
};

static const float2 gTexC[4] =
{
	float2(0.f, 1.f),
	float2(0.f, 0.f),
	float2(1.f, 1.f),
	float2(1.f, 0.f)
};

[maxvertexcount(4)]
void main(
	point GSInput input[1], 
	inout TriangleStream< GSOutput > output
)
{
	float3 up = float3(0.f, 1.f, 0.f);
	float3 look = gEyePos - input[0].CenterW;
	look.y = 0.f;//y-axis aligned
	look = normalize(look);
	float3 right = cross(up, look);

	float halfWidth = 0.5f*input[0].SizeW.x;
	float halfHeight = 0.5f*input[0].SizeW.y;

	float4 v[4];

	v[0] = float4(input[0].CenterW + halfWidth*right - halfHeight*up, 1.0f); 
	v[1] = float4(input[0].CenterW + halfWidth*right + halfHeight*up, 1.0f); 
	v[2] = float4(input[0].CenterW - halfWidth*right - halfHeight*up, 1.0f); 
	v[3] = float4(input[0].CenterW - halfWidth*right + halfHeight*up, 1.0f);

	GSOutput gout;
	[unroll]
	for (uint i = 0; i < 4; i++)
	{
		gout.PosH = mul(v[i], gViewProj);
		gout.PosW = v[i].xyz;
		gout.NormalW = look;//toward camera!
		gout.Tex = gTexC[i];
		output.Append(gout);
	}
}