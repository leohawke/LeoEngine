float2 fade(float2 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10); // new curve (quintic)
}

//the fuck hlsl ,template please!!!!!!

float3 mod289(float3 x){
	return x - floor(x*(1.0 / 289.0))*289.0;
}

float2 mod289(float2 x){
	return x - floor(x*(1.0 / 289.0))*289.0;
}

float3 permute(float3 x){
	return mod289(((x*34.0) + 1.0)*x);
}


float fract(float x){
	return x - floor(x);
}

float2 fract(float2 x){
	return x - floor(x);
}

float3 fract(float3 x){
	return x - floor(x);
}

float inoise(float2 v){
	const float4 C = float4(0.21132486540517, 0.366025403784439, -0.577350269189626, 0.024390243902439);

	float2 i = floor(v + dot(v, C.yy));
		float2 x0 = v - i + dot(i, C.xx);

		// Other corners
		float2 i1;
	//i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
	//i1.y = 1.0 - i1.x;
	i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
	// x0 = x0 - 0.0 + 0.0 * C.xx ;
	// x1 = x0 - i1 + 1.0 * C.xx ;
	// x2 = x0 - 1.0 + 2.0 * C.xx ;
	float4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	// Permutations
	i = mod289(i); // Avoid truncation effects in permutation
	float3 p = permute(permute(i.y + float3(0.0, i1.y, 1.0))
		+ i.x + float3(0.0, i1.x, 1.0));

	float3 m = max(0.5 - float3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
	m = m*m;
	m = m*m;

	// Gradients: 41 points uniformly over a line, mapped onto a diamond.
	// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

	float3 x = 2.0 * fract(p * C.www) - 1.0;
	float3 h = abs(x) - 0.5;
	float3 ox = floor(x + 0.5);
	float3 a0 = x - ox;

	// Normalise gradients implicitly by scaling m
	// Approximation of: m *= inversesqrt( a0*a0 + h*h );
	m *= 1.79284291400159 - 0.85373472095314 * (a0*a0 + h*h);

	// Compute final noise value at P
	float3 g;
	g.x = a0.x  * x0.x + h.x  * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);

}

#if 0
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
#endif

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