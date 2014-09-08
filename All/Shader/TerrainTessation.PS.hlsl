#include "TerrainTessationCommon.hlsli"

SamplerState RepeatMaxAniso :register(s2)
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};
SamplerState RepeatMedAniso :register(s3)
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	AddressU = Wrap;
	AddressV = Wrap;
};
SamplerState RepeatPoint:register(s4)
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
// Smooth shading pixel shader section
//--------------------------------------------------------------------------------------
Texture2D gTerrainColourTexture1 : register(t2);
Texture2D gTerrainColourTexture2 : register(t3);
Texture2D gDetailNoiseGradTexture : register(t4);
Texture2D gCoarseGradientMap : register(t5);
Texture2D gNoiseTexture : register(t6);
cbuffer ParamOnSet : register(b1)
{
	float3 gFractalOctaves;
}

#ifdef DEBUG
cbuffer DebugOnSet : register(b2)
{
	bool gDebugShowPatches = false;
}
#endif
// A very simple, regular procedural terrain for debugging cracks etc.
float debugSineHills(float2 uv)
{
	const float HORIZ_SCALE = 4 * 3.14159, VERT_SCALE = 1;
	uv *= HORIZ_SCALE;
	return VERT_SCALE * (sin(uv.x) + 1) * (sin(uv.y) + 1) - 0.5;
}

float DetailUVScale()
{
	const float DETAIL_UV_SCALE = pow(2, gFractalOctaves.x + gFractalOctaves.y + gFractalOctaves.z - 5);
	return DETAIL_UV_SCALE;
}


float2 ScaleDetailGrad(float2 grad)
{
	return gDetailNoiseScale * ((2 * grad) - 1);		// 2x-1 scale and bias
}



float2 SampleDetailGradOctaves(float2 uv)
{
	// There is a 50x scale built into the texture in order to make better use of the range.  This scale
	// is hardcoded by the GUI in the normal map generator.  Compensate here with a 1/50.
	const float TEX_SCALE = 0.02;
	float2 grad = TEX_SCALE * ScaleDetailGrad(gDetailNoiseGradTexture.Sample(RepeatMaxAniso, DetailNoiseSampleCoords(uv)).ra);
		return grad;
}

float HighContrast(float i)
{
	//return 1.3 * (i-0.1);
	//return 1.9 * (i-0.2); // higher
	return i;
}


float3 SampleDetailNormal(float2 worldXZ)
{
	// The detail displacement is scaled by the coarse displacement's height.  So we need 
	// to fetch the height to scale the normal.  (The scale computation could perhaps be
	// moved up to the DS.)
	const float2 uv = worldXZtoHeightUV(worldXZ);
	float coarse = gCoarseHeightMap.Sample(ClampLinear, uv).r;

	const float vScale = saturate(coarse) * WORLD_SCALE * VERTICAL_SCALE;

	// The MIP-mapping doesn't seem to work very well.  Maybe I need to think more carefully about
	// anti-aliasing the normal function?
	float2 grad = SampleDetailGradOctaves(uv);
		return normalize(float3(-vScale * grad.x,gCoarseSampleSpacing * WORLD_UV_REPEATS_RECIP * gDetailUVScale.y, vScale * grad.y));
}

float DebugCracksPattern(MeshVertex input)
{
	// Dark grey and black - to better show any cracks.
	if (gDebugShowPatches)
		return 0.1 * (input.vNormal.x - 0.5);
	else
		return 0;
}

float4 SmoothShadePS(MeshVertex input) : SV_Target
{
	//return DebugCracksPattern(input);

	// Not sure about the arbitrary 2x.  It looks right visually.  Maybe there's a constant somewhere in
	// the texture sizes that I overlooked?!?
	const float ARBITRARY_FUDGE = 2;
	const float2 grad =gCoarseGradientMap.Sample(RepeatLinear, worldXZtoHeightUV(input.vWorldXZ)).rg;
	const float vScale = ARBITRARY_FUDGE * gfDisplacementHeight * WORLD_SCALE * VERTICAL_SCALE;
	const float3 coarseNormal = normalize(float3(vScale * grad.x,gCoarseSampleSpacing, -vScale * grad.y));
	const float3 detailNormal = SampleDetailNormal(input.vWorldXZ);
	const float3 normal = normalize(coarseNormal + detailNormal);

	// Texture coords have to be offset by the eye's 2D world position.  Why the 2x???
	const float2 texUV = input.vWorldXZ + 2 * float2(gTextureWorldOffset.x, -gTextureWorldOffset.z);;

	// We apply two textures at vastly different scales: macro and micro detail.
	float3 macroDetail = gTerrainColourTexture1.Sample(RepeatMaxAniso, texUV).xyz;				// we know that this is grey only
		float4 microDetail = gTerrainColourTexture2.Sample(RepeatMedAniso, texUV * 50);
		float  gaussian = gNoiseTexture.Sample(RepeatPoint, texUV * 50.0 / 256.0).x;
	float  uniformRandom = frac(gaussian * 10);
	float  randomizedDetail = 1;

	// Randomly choose between four versions of the micro texture in RGBA.  This sort of 
	// blending is representative of game-engine terrain shaders (and the perf).
	if (uniformRandom >= 0.75)
		randomizedDetail = microDetail.x;
	else if (uniformRandom >= 0.5)
		randomizedDetail = microDetail.y;
	else if (uniformRandom >= 0.25)
		randomizedDetail = microDetail.z;
	else
		randomizedDetail = microDetail.w;

	// Some of the lunar reference photos have a brownish tint.  IMO, it looks slightly better here.
	const float3 colour = float3(0.988, 0.925, 0.847);

	// This light direction approximately matches the pre-baked direction in the NASA photos.
	const float3 lightDir = normalize(float3(-0.74, 0.45, -0.15));
	float lit = saturate(HighContrast(dot(lightDir, normal)));

	// Chequer pattern still comes down from the DS.
	if (gDebugShowPatches)
		lit *= input.vNormal.x;

	//return lit;
	return float4(colour * macroDetail * randomizedDetail * lit, 1);
}