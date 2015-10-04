static const int    MAX_SAMPLES = 64;    // Maximum texture grabs
Texture2D mSrc;

cbuffer SampleCB :register(c0){
	float4 g_avSampleOffsets[MAX_SAMPLES/2];
}

SamplerState s0 : register(s0);

float4 main_2(in float2 Tex :TEXCOORD) : SV_TARGET
{
	float4 sample = 0.0f;

	float2 avSampleOffsets[MAX_SAMPLES] = g_avSampleOffsets;
	for (int i = 0; i < 4; i++)
	{
		sample += mSrc.Sample(s0, Tex + avSampleOffsets[i]);
	}

	return sample / 4;
}

float4 main_4(in float2 Tex :TEXCOORD) : SV_TARGET
{
	float4 sample = 0.0f;

	float2 avSampleOffsets[MAX_SAMPLES] = g_avSampleOffsets;
	for (int i = 0; i < 16; i++)
	{
		sample += mSrc.Sample(s0, Tex + avSampleOffsets[i]);
	}

	return sample / 16;
}

float4 main_8(in float2 Tex :TEXCOORD) : SV_TARGET
{
	float4 sample = 0.0f;
	float2 avSampleOffsets[MAX_SAMPLES] = g_avSampleOffsets;
	for (int i = 0; i < 64; i++)
	{
		sample += mSrc.Sample(s0, Tex + avSampleOffsets[i]);
	}

	return sample / 64;
}