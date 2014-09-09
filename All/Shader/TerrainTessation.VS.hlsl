#include "TerrainTessationCommon.hlsli"



struct VertexIn
{
	float2 position  : POSITION_2D;
	Adjacency adjacency;
	uint VertexId    : SV_VertexID;
	uint InstanceId  : SV_InstanceID;
};

struct VS_CONTROL_POINT_OUTPUT
{
	float3 vPosition        : POSITION;
	float2 vWorldXZ         : TEXCOORD1;
	Adjacency adjacency : ADJACENCY_SIZES;
};


cbuffer SizeAndOnSet : register(b1)
{
	float gTileSize = 1;
#ifdef DEBUG
	bool gDebugShowTiles = false;
#endif
}

// There's no vertex data.  Position and UV coords are created purely from vertex and instance IDs.
void ReconstructPosition(VertexIn input, out float3 pos, out int2 intUV)
{
	float iv = floor(input.VertexId * RECIP_CONTROL_VTX_PER_TILE_EDGE);
	float iu = input.VertexId - iv * CONTROL_VTX_PER_TILE_EDGE;
	float u = iu / (CONTROL_VTX_PER_TILE_EDGE - 1.0);
	float v = iv / (CONTROL_VTX_PER_TILE_EDGE - 1.0);

	// Shrink tiles slightly to show gaps between them.
	float size = gTileSize;
#ifdef DEBUG
	if (gDebugShowTiles)
		size *= 0.98;
#endif
	pos = float3(u * size + input.position.x, 0, v * size + input.position.y);
	intUV = int2(iu, iv);
}

void ReconstructPosition(VertexIn input, out float3 pos)
{
	int2 dummy;
	ReconstructPosition(input, pos, dummy);
}

VS_CONTROL_POINT_OUTPUT HwTessellationPassThruVS(VertexIn input)
{
	VS_CONTROL_POINT_OUTPUT output;
	int2 intUV;
	ReconstructPosition(input, output.vPosition, intUV);

	float z = SampleHeightForVS(gCoarseHeightMap, ClampLinear, output.vPosition.xz);
	output.vPosition.y += z;
	output.vWorldXZ = output.vPosition.xz;
	output.adjacency = input.adjacency;

	return output;
}