#include "TerrainTessationCommon.hlsli"
#ifdef DEBUG
float3 debugCubes(float2 uv)
{
	const float HORIZ_SCALE = 4, VERT_SCALE = 1;
	uv *= HORIZ_SCALE;
	return VERT_SCALE * floor(fmod(uv.x, 2.0)) * floor(fmod(uv.y, 2.0));
}

float3 debugXRamps(float2 uv)
{
	const float HORIZ_SCALE = 2, VERT_SCALE = 1;
	uv *= HORIZ_SCALE;
	return VERT_SCALE * frac(uv.x);
}

float3 debugSineHills(float2 uv)
{
	const float HORIZ_SCALE = 2 * 3.14159, VERT_SCALE = 0.5;
	uv *= HORIZ_SCALE;
	//uv += 2.8;			// arbitrarily not centered - test asymetric fns.
	return VERT_SCALE * (sin(uv.x) + 1) * (sin(uv.y) + 1);
}

float3 debugFlat(float2 uv)
{
	const float VERT_SCALE = 0.1;
	return VERT_SCALE;
}
#endif

struct DeformVertex
{
	float4 pos : SV_Position;
	float2 texCoord : TEXCOORD1;
};

float4 InitializationPS(DeformVertex input) : SV_Target
{
	const float2 uv = gTextureWorldOffset.xz + WORLD_UV_REPEATS * input.texCoord;
#ifdef DEBUG
#ifdef XRamps
	return float4(debugXRamps(uv),  1);
#endif
#ifdef Flat
	return float4(debugFlat(uv), 1);
#endif
#ifdef SineHills
	return float4(debugSineHills(uv),  1);
#endif
#ifdef Cubes
	return float4(debugCubes(uv), 1);
#endif
#else
	return hybridTerrain(uv, mFractalOctaves);
#endif
}