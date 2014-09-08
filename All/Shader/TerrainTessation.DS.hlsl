#include "TerrainTessationCommon.hlsli"

cbuffer ChangeOnCamera : register(b1)
{
	float4x4 gWorldViewProj;
}

#ifdef DEBUG
cbuffer DebugOnSet : register(b2)
{
	bool gDebugShowPatches = false;
}
#endif
float3 bilerpColour(float3 c0, float3 c1, float3 c2, float3 c3, float2 UV)
{
	float3 left = lerp(c0, c1, UV.y);
		float3 right = lerp(c2, c3, UV.y);
		float3 result = lerp(left, right, UV.x);
		return result;
}

float3 lerpDebugColours(float3 cIn[5], float2 uv)
{
	if (uv.x < 0.5 && uv.y < 0.5)
		return bilerpColour(0.5* (cIn[0] + cIn[1]), cIn[0], cIn[1], cIn[4], 2 * uv);
	else if (uv.x < 0.5 && uv.y >= 0.5)
		return bilerpColour(cIn[0], 0.5* (cIn[0] + cIn[3]), cIn[4], cIn[3], 2 * (uv - float2(0, 0.5)));
	else if (uv.x >= 0.5 && uv.y < 0.5)
		return bilerpColour(cIn[1], cIn[4], 0.5* (cIn[2] + cIn[1]), cIn[2], 2 * (uv - float2(0.5, 0)));
	else // x >= 0.5 && y >= 0.5
		return bilerpColour(cIn[4], cIn[3], cIn[2], 0.5* (cIn[2] + cIn[3]), 2 * (uv - float2(0.5, 0.5)));
}

// Templates please!!!
float2 Bilerp(float2 v0, float2 v1, float2 v2, float2 v3, float2 i)
{
	float2 bottom = lerp(v0, v3, i.x);
		float2 top = lerp(v1, v2, i.x);
		float2 result = lerp(bottom, top, i.y);
		return result;
}

float3 Bilerp(float3 v0, float3 v1, float3 v2, float3 v3, float2 i)
{
	float3 bottom = lerp(v0, v3, i.x);
		float3 top = lerp(v1, v2, i.x);
		float3 result = lerp(bottom, top, i.y);
		return result;
}

struct HS_CONSTANT_DATA_OUTPUT
{
	float Edges[4]        : SV_TessFactor;
	float Inside[2]       : SV_InsideTessFactor;
	float2 vWorldXZ[4]    : TEXCOORD4;
	float3 debugColour[5] : COLOR;			// 5th is centre
};

struct HS_OUTPUT
{
	float3 vPosition : POSITION;
};

float3 TessellatedWorldPos(HS_CONSTANT_DATA_OUTPUT input,
	float2 UV : SV_DomainLocation,
	const OutputPatch<HS_OUTPUT, 4> terrainQuad)
{
	// bilerp the position
	float3 worldPos = Bilerp(terrainQuad[0].vPosition, terrainQuad[1].vPosition, terrainQuad[2].vPosition, terrainQuad[3].vPosition, UV);

		const int mipLevel = 0;

	float height = SampleHeightForVS(gCoarseHeightMap, ClampLinear, worldPos.xz);
	worldPos.y += gfDisplacementHeight * height;

	return worldPos;
}

// The domain shader is run once per vertex and calculates the final vertex's position
// and attributes.  It receives the UVW from the fixed function tessellator and the
// control point outputs from the hull shader.  Since we are using the DirectX 11
// Tessellation pipeline, it is the domain shader's responsibility to calculate the
// final SV_POSITION for each vertex.

// The input SV_DomainLocation to the domain shader comes from fixed function
// tessellator.  And the OutputPatch comes from the hull shader.  From these, you
// must calculate the final vertex position, color, texcoords, and other attributes.

// The output from the domain shader will be a vertex that will go to the video card's
// rasterization pipeline and get drawn to the screen.
[domain("quad")]
MeshVertex TerrainDisplaceDS(HS_CONSTANT_DATA_OUTPUT input,
	float2 UV : SV_DomainLocation,
	const OutputPatch<HS_OUTPUT, 4> terrainQuad,
	uint PatchID : SV_PrimitiveID)
{
	MeshVertex Output = (MeshVertex)0;

	const float3 worldPos = TessellatedWorldPos(input, UV, terrainQuad);
	Output.vPosition = mul(float4(worldPos.xyz, 1), gWorldViewProj);
#ifdef DEBUG
	Output.debugColour = lerpDebugColours(input.debugColour, UV);
#endif
	Output.vWorldXZ = worldPos.xz;
	Output.vNormal = float3(1, 1, 1);
#ifdef DEBUG
	// For debugging, darken a chequer board pattern of tiles to highlight tile boundaries.
	if (gDebugShowPatches)
	{
		const uint patchY = PatchID / PATCHES_PER_TILE_EDGE;
		const uint patchX = PatchID - patchY * PATCHES_PER_TILE_EDGE;
		Output.vNormal *= (0.5 * ((patchX + patchY) % 2) + 0.5);
	}
#endif
	return Output;
}
