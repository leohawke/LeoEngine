float2 fade(float2 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10); // new curve (quintic)
}

float inoise(float2 p)
{
	float2 i = floor(p * 256);
	float2 f = 256 * p - i;
	f = fade(f);
	i /= 256;


	float4 n;
	const float mipLevel = 0;
	n.x = gNoiseTexure.SampleLevel(RepeatPoint, i, mipLevel).x;
	n.y = gNoiseTexure.SampleLevel(RepeatPoint, i, mipLevel, int2(1, 0)).x;
	n.z = gNoiseTexure.SampleLevel(RepeatPoint, i, mipLevel, int2(0, 1)).x;
	n.w = gNoiseTexure.SampleLevel(RepeatPoint, i, mipLevel, int2(1, 1)).x;

	const float interpolated = lerp(lerp(n.x, n.y, f.x),
		lerp(n.z, n.w, f.x), f.y);	// [0,1]
	return 2.0 * interpolated - 1.0;
}


float fBm(float2 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
{
	float freq = 1.0, amp = 1.0;
	float sum = 0;
	for (int i = 0; i<octaves; i++) {
		sum += inoise(p*freq)*amp;
		freq *= lacunarity;
		amp *= gain;
	}
	return sum;
}

float ridge(float h, float offset)
{
	h = abs(h);
	h = offset - h;
	h = h * h;
	return h;
}

float ridgedmf(float2 p, int octaves, float lacunarity = 2.0, float gain = 0.5, float offset = 1.0)
{
	// Hmmm... these hardcoded constants make it look nice.  Put on tweakable sliders?
	float f = 0.3 + 0.5 * fBm(p, octaves, lacunarity, gain);
	return ridge(f, offset);
}

// fractal sum


// mixture of ridged and fbm noise
float hybridTerrain(float2 x, int3 octaves)
{
	const float SCALE = 256;
	x /= SCALE;

	const int RIDGE_OCTAVES = octaves.x;
	const int FBM_OCTAVES = octaves.y;
	const int TWIST_OCTAVES = octaves.z;
	const float LACUNARITY = 2, GAIN = 0.5;

	// Distort the ridge texture coords.  Otherwise, you see obvious texel edges.
	float2 xOffset = float2(fBm(0.2*x, TWIST_OCTAVES), fBm(0.2*x + 0.2, TWIST_OCTAVES));
		float2 xTwisted = x + 0.01 * xOffset;

		// Ridged is too ridgy.  So interpolate between ridge and fBm for the coarse octaves.
		float h = ridgedmf(xTwisted, RIDGE_OCTAVES, LACUNARITY, GAIN, 1.0);

	const float fBm_UVScale = pow(LACUNARITY, RIDGE_OCTAVES);
	const float fBm_AmpScale = pow(GAIN, RIDGE_OCTAVES);
	float f = fBm(x * fBm_UVScale, FBM_OCTAVES, LACUNARITY, GAIN) * fBm_AmpScale;

	if (RIDGE_OCTAVES > 0)
		return h + f*saturate(h);
	else
		return f;
}