#ifndef TerrainTessationCommon_HLSLI
#define TerrainTessationCommon_HLSLI

static const int CONTROL_VTX_PER_TILE_EDGE = 9;
static const float RECIP_CONTROL_VTX_PER_TILE_EDGE = 1.0 / 9;
static const int PATCHES_PER_TILE_EDGE = 8;
//Change the values for different  types of games
static const float WORLD_SCALE = 400;
static const float VERTICAL_SCALE = 0.65;
static const float WORLD_UV_REPEATS = 8;	// How many UV repeats across the world for fractal generation.
static const float WORLD_UV_REPEATS_RECIP = 1.0 / WORLD_UV_REPEATS;

cbuffer ScaleAndTexOffsetOnSet : register(b0)
{
	float3 gTextureWorldOffset : packoffset(c0);	// Offset of fractal terrain in texture space.
	float     gDetailNoiseScale : packoffset(c0.w);
	float2    gDetailUVScale : packoffset(c1);				// x is scale; y is 1/scale
		float  gCoarseSampleSpacing : packoffset(c1.z);
	float gfDisplacementHeight : packoffset(c1.w);
}

Texture2D gCoarseHeightMap : register(t0);
SamplerState ClampLinear : register(s0)
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

Texture2D gDetailNoiseTexture :register(t1);
SamplerState RepeatLinear : register(s1)
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Repeat;
	AddressV = Repeat;
};




float2 worldXZtoHeightUV(float2 worldXZ)
{
	// [-8,8] -> [0,1]  TBD: Ought to depend on world size though.
	return worldXZ / 16 + 0.5;
}

float ScaleDetailNoise(float coarse, float detail)
{
	// The 8-bit texture needs a scale + bias.
	detail = 2.0 * detail - 1.0;

	// Note the detail is modulated by the height of the coarse sample, ridge octave style.
	return gDetailNoiseScale * detail * saturate(coarse);
}

float2 DetailNoiseSampleCoords(float2 uv)
{
	// Texture coords have to be offset by the eye's 2D world position.
	const float2 texOffset = float2(gTextureWorldOffset.x, -gTextureWorldOffset.z);

	const float2 detailUV = (texOffset + WORLD_UV_REPEATS * uv) * gDetailUVScale.x;		// TBD: is WORLD_UV_REPEATS spurious here?
	return detailUV;
}

float SampleLevelDetailNoise(float2 uv, float coarse)
{
	const int mip = 0;
	float detail = gDetailNoiseTexture.SampleLevel(RepeatLinear, DetailNoiseSampleCoords(uv), mip).r;
	return ScaleDetailNoise(coarse, detail);
}

// Wrappers for displacement map sampling allow us to substitute a 100% procedural surface.
// VS and DS sampling have to agree and use common code.
float SampleHeightForVS(Texture2D coarseTex, SamplerState coarseSampler, float2 worldXZ, int2 offset)
{
	const float2 uv = worldXZtoHeightUV(worldXZ);
	const int mipLevel = 0;

	// You can implement purely procedural terrain here, evaluating the height fn on-the-fly.
	// But for any complex function, with loads of octaves, it's faster to sample from a texture.
	// return debugSineHills(uvCol);						// Simple test shape.
	// return 1.5 * hybridTerrain(uvCol, g_FractalOctaves);	// Procedurally sampled fractal terrain.

	// Fractal terrain sampled from texture.  This is a render target that is updated when the view 
	// point changes.  There are two levels of textures here.  1) The low-res ridge noise map that is 
	// initialized by the deformation effect.  2) A detail texture that contains 5 octaves of 
	// repeating fBm.
	float coarse = coarseTex.SampleLevel(coarseSampler, uv, mipLevel, offset).r;	// coarse

	return VERTICAL_SCALE * (coarse + SampleLevelDetailNoise(uv, coarse));		// detail
}

float SampleHeightForVS(Texture2D tex, SamplerState sam, float2 worldXZ)
{
	int2 offset = 0;
		return SampleHeightForVS(tex, sam, worldXZ, offset);
}

//pack float4
struct Adjacency
{
	// These are the size of the neighbours along +/- x or y axes.  For interior tiles
	// this is 1.  For edge tiles it is 0.5 or 2.0.
	float neighbourMinusX : ADJACENCY_SIZES0;
	float neighbourMinusY : ADJACENCY_SIZES1;
	float neighbourPlusX : ADJACENCY_SIZES2;
	float neighbourPlusY : ADJACENCY_SIZES3;
};

struct MeshVertex
{
	float4 vPosition        : SV_Position;
	float2 vWorldXZ         : TEXCOORD1;
	float3 vNormal          : NORMAL;
#ifdef DEBUG
	float3 debugColour      : COLOR;
#endif
};
#endif