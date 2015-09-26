static const int    MAX_SAMPLES = 64;    // Maximum texture grabs
Texture2D mSrc;

cbuffer SampleCB :register(c0) {
	float4 g_avSampleOffsets[MAX_SAMPLES / 2];
}

SamplerState s0 : register(s0);

float4 EncodeFloatRGBA(float v)
{
	float4 enc = float4(1.0f, 255.0f, 65025.0f, 16581375.0f) * v;
	enc = frac(enc);
	enc -= enc.yzww * float4(1 / 255.0f, 1 / 255.0f, 1 / 255.0f, 0);
	return enc;
}

float DecodeFloatRGBA(float4 rgba)
{
	return dot(rgba, float4(1, 1 / 255.0f, 1 / 65025.0f, 1 / 16581375.0f));
}

float4 LumIterative(
	in float4 PosH:SV_POSITION,
	in float2 Tex : TEXCOORD
	) : SV_TARGET
{
	float fResampleSum = 0.0f;
	float2 avSampleOffsets[MAX_SAMPLES] = g_avSampleOffsets;
	for (int iSample = 0; iSample < 16; iSample++)
	{
		// Compute the sum of luminance throughout the sample points
		#ifdef NO_SINGLE_CHANNEL_FLOAT
		fResampleSum += DecodeFloatRGBA(mSrc.Sample(s0, Tex + avSampleOffsets[iSample]).xyzw);
		#else
		fResampleSum += mSrc.Sample(s0, Tex + avSampleOffsets[iSample]).x;
		#endif
	}

	// Divide the sum to complete the average
	fResampleSum /= 16;

	#ifdef NO_SINGLE_CHANNEL_FLOAT
	return EncodeFloatRGBA(fResampleSum);
	#else
	return fResampleSum;
	#endif
}