
struct VSInput
{
	float3 CenterW : POSITION;
	float2 SizeW : SIZE;
};

struct VSOutput
{
	float3 CenterW : POSITION;
	float2 SizeW : SIZE;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	output.CenterW = input.CenterW;
	output.SizeW = input.SizeW;
	return output;
}