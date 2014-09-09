#include "TerrainTessationCommon.hlsli"

cbuffer ChangeOnHardWare : register(b1)
{
	//Render target size for screen-space calculations
	float2 gScreenSize;
	//Pixels per tri edge
	int gTessellatedTriWidth = 10;
}

cbuffer ChangeOnMatrix : register(b2)
{
	row_major float4x4 gWorldViewProj;

	row_major float4x4 gWorldViewLOD;
	row_major float4x4 gWorldViewProjLOD;

	//The proj matrix does not vary between the LOD and view-centre vsets.  
	//Only the view matrix varies
	row_major float4x4 gProj;

	float3 gEyePos;
	float3 gViewDir;
}

float ClipToScreenSpaceTessellation(float4 clip0, float4 clip1)
{
	clip0 /= clip0.w;
	clip1 /= clip1.w;

	clip0.xy *= gScreenSize;
	clip1.xy *= gScreenSize;

	const float d = distance(clip0, clip1);
	return clamp(d / gTessellatedTriWidth, 0, 64);
}

// Project an edge into clip space and return the number of triangles that are required to fit across it
// in screenspace.
float EdgeToScreenSpaceTessellation(float3 p0, float3 p1)
{
	float4 clip0 = mul(float4(p0, 1), gWorldViewProjLOD);
	float4 clip1 = mul(float4(p1, 1), gWorldViewProjLOD);
	return ClipToScreenSpaceTessellation(clip0, clip1);
}

float SphereToScreenSpaceTessellation(float3 p0, float3 p1, float diameter)
{
	float3 centre = 0.5 * (p0 + p1);
	float4 view0 = mul(float4(centre, 1), gWorldViewLOD);
	float4 view1 = view0;
	view1.x += WORLD_SCALE * diameter;

	float4 clip0 = mul(view0, gProj);
	float4 clip1 = mul(view1, gProj);
	return ClipToScreenSpaceTessellation(clip0, clip1);
}

// Lifted from Tim's Island demo code.
bool inFrustum(const float3 pt, const float3 eyePos, const float3 viewDir, float margin)
{
	// conservative frustum culling
	float3 eyeToPt = pt - eyePos;
		float3 patch_to_camera_direction_vector = viewDir * dot(eyeToPt, viewDir) - eyeToPt;
		float3 patch_center_realigned = pt + normalize(patch_to_camera_direction_vector) * min(margin, length(patch_to_camera_direction_vector));
		float4 patch_screenspace_center = mul(float4(patch_center_realigned, 1.0), gWorldViewProjLOD);

		if (((patch_screenspace_center.x / patch_screenspace_center.w > -1.0) && (patch_screenspace_center.x / patch_screenspace_center.w < 1.0) &&
			(patch_screenspace_center.y / patch_screenspace_center.w > -1.0) && (patch_screenspace_center.y / patch_screenspace_center.w < 1.0) &&
			(patch_screenspace_center.w>0)) || (length(pt - eyePos) < margin))
		{
		return true;
		}

	return false;
}

// The adjacency calculations ensure that neighbours have tessellations that agree.
// However, only power of two sizes *seem* to get correctly tessellated with no cracks.
float SmallerNeighbourAdjacencyClamp(float tess)
{
	// Clamp to the nearest larger power of two.  Any power of two works; larger means that we don't lose detail.
	// Output is [4,64].
	float logTess = ceil(log2(tess));
	float t = pow(2, logTess);

	// Our smaller neighbour's min tessellation is pow(2,1) = 2.  As we are twice its size, we can't go below 4.
	return max(4, t);
}

float LargerNeighbourAdjacencyClamp(float tess)
{
	// Clamp to the nearest larger power of two.  Any power of two works; larger means that we don't lose detail.
	float logTess = ceil(log2(tess));
	float t = pow(2, logTess);

	// Our larger neighbour's max tessellation is 64; as we are half its size, our tessellation must max out
	// at 32, otherwise we could be over-tessellated relative to the neighbour.  Output is [2,32].
	return clamp(t, 2, 32);
}

void MakeVertexHeightsAgree(inout float3 p0, inout float3 p1)
{
	// This ought to work: if the adjacency has repositioned a vertex in XZ, we need to re-acquire its height.
	// However, causes an internal fxc error.  Again! :-(
	//float h0 = SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, p0.xz);
	//float h1 = SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, p0.xz);
	//p0.y = h0;
	//p1.y = h1;

	// Instead set both vertex heights to zero.  It's the only way I can think to agree with the neighbours
	// when sampling is broken in fxc.
	p0.y = p1.y = 0;
}

float SmallerNeighbourAdjacencyFix(float3 p0, float3 p1, float diameter)
{
	MakeVertexHeightsAgree(p0, p1);
	float t = SphereToScreenSpaceTessellation(p0, p1, diameter);
	return SmallerNeighbourAdjacencyClamp(t);
}

float LargerNeighbourAdjacencyFix(float3 p0, float3 p1, int patchIdx, float diameter)
{
	// We move one of the corner vertices in 2D (x,z) to match where the corner vertex is 
	// on our larger neighbour.  We move p0 or p1 depending on the even/odd patch index.
	//
	// Larger neighbour
	// +-------------------+
	// +---------+
	// p0   Us   p1 ---->  +		Move p1
	// |    0    |    1    |		patchIdx % 2 
	//
	//           +---------+
	// +  <----  p0   Us   p1		Move p0
	// |    0    |    1    |		patchIdx % 2 
	//
	if (patchIdx % 2)
		p0 += (p0 - p1);
	else
		p1 += (p1 - p0);

	// Having moved the vertex in (x,z), its height is no longer correct.
	MakeVertexHeightsAgree(p0, p1);

	// Half the tessellation because the edge is twice as long.
	float t = 0.5 * SphereToScreenSpaceTessellation(p0, p1, 2 * diameter);
	return LargerNeighbourAdjacencyClamp(t);
}

struct VS_CONTROL_POINT_OUTPUT
{
	float3 vPosition        : POSITION;
	float2 vWorldXZ         : TEXCOORD1;
	Adjacency adjacency : ADJACENCY_SIZES;
};

//--------------------------------------------------------------------------------------
// Constant data function for the Terrain tessellation.  This is executed once per patch.
//--------------------------------------------------------------------------------------
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

// These are one colour for each tessellation level and linear graduations between.
static const float3 DEBUG_COLOURS[6] =
{
	float3(0, 0, 1),  //  2 - blue
	float3(0, 1, 1),	//  4 - cyan
	float3(0, 1, 0),	//  8 - green
	float3(1, 1, 0),	// 16 - yellow
	float3(1, 0, 1),	// 32 - purple
	float3(1, 0, 0),	// 64 - red
};

// This constant hull shader is executed once per patch.
HS_CONSTANT_DATA_OUTPUT TerrainScreenspaceLODConstantsHS(InputPatch<VS_CONTROL_POINT_OUTPUT, 4> ip, uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output = (HS_CONSTANT_DATA_OUTPUT)0;

	const float3 centre = 0.25 * (ip[0].vPosition + ip[1].vPosition + ip[2].vPosition + ip[3].vPosition);
	const float  sideLen = max(abs(ip[1].vPosition.x - ip[0].vPosition.x), abs(ip[1].vPosition.x - ip[2].vPosition.x));		// assume square & uniform
	const float  diagLen = sqrt(2 * sideLen*sideLen);

	if (!inFrustum(centre, gEyePos / WORLD_SCALE, gViewDir, diagLen))
	{
		Output.Inside[0] = Output.Inside[1] = -1;
		Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = -1;
	}
	else
	{
		// Alternative implementation left here for instruction purposes:
		// Examining displaced patch corners completely fails for patches that are seen edge-on where the patch has
		// significant interior displacement.  In screenspace, two patch dimensions are ~= 0.  However, once displaced
		// in the DS the screenspace size is significantly > 0.  I think this edge-based version simply doesn't work
		// well unless you have a good estimate of the max interior displacement (needs a max displacement per patch).
		//Output.Edges[0] = EdgeToScreenSpaceTessellation(ip[0].vPosition, ip[1].vPosition);
		//Output.Edges[3] = EdgeToScreenSpaceTessellation(ip[1].vPosition, ip[2].vPosition);
		//Output.Edges[2] = EdgeToScreenSpaceTessellation(ip[2].vPosition, ip[3].vPosition);
		//Output.Edges[1] = EdgeToScreenSpaceTessellation(ip[3].vPosition, ip[0].vPosition);

		// Alternatively: project a sphere centered on the patch edge mid-point (not patch bounding sphere - that
		// wouldn't work with adjacency fixes, etc).  This is independent of patch orientation and doesn't suffer 
		// from edges getting projected to zero size.  The flip side is that it over-tessellates flat, boring areas 
		// and uses more polygons overall.
		Output.Edges[0] = SphereToScreenSpaceTessellation(ip[0].vPosition, ip[1].vPosition, sideLen);
		Output.Edges[3] = SphereToScreenSpaceTessellation(ip[1].vPosition, ip[2].vPosition, sideLen);
		Output.Edges[2] = SphereToScreenSpaceTessellation(ip[2].vPosition, ip[3].vPosition, sideLen);
		Output.Edges[1] = SphereToScreenSpaceTessellation(ip[3].vPosition, ip[0].vPosition, sideLen);

		// Edges that need adjacency adjustment are identified by the per-instance ip[0].adjacency 
		// scalars, in *conjunction* with a patch ID that puts them on the edge of a tile.
		int2 patchXY;
		patchXY.y = PatchID / PATCHES_PER_TILE_EDGE;
		patchXY.x = PatchID - patchXY.y * PATCHES_PER_TILE_EDGE;

		// Identify patch edges that are adjacent to a patch of a different size.  The size difference
		// is encoded in ip[n].adjacency, either 0.5, 1.0 or 2.0.
		// neighbourMinusX refers to our adjacent neighbour in the direction of -ve x.  The value 
		// is the neighbour's size relative to ours.  Similarly for plus and Y, etc.  You really
		// need a diagram to make sense of the adjacency conditions in the if statements. :-(
		// These four ifs deal with neighbours that are smaller.
		if (ip[0].adjacency.neighbourMinusX < 0.55 && patchXY.x == 0)
			Output.Edges[0] = SmallerNeighbourAdjacencyFix(ip[0].vPosition, ip[1].vPosition, sideLen);
		if (ip[0].adjacency.neighbourMinusY < 0.55 && patchXY.y == 0)
			Output.Edges[1] = SmallerNeighbourAdjacencyFix(ip[3].vPosition, ip[0].vPosition, sideLen);
		if (ip[0].adjacency.neighbourPlusX < 0.55 && patchXY.x == PATCHES_PER_TILE_EDGE - 1)
			Output.Edges[2] = SmallerNeighbourAdjacencyFix(ip[2].vPosition, ip[3].vPosition, sideLen);
		if (ip[0].adjacency.neighbourPlusY < 0.55 && patchXY.y == PATCHES_PER_TILE_EDGE - 1)
			Output.Edges[3] = SmallerNeighbourAdjacencyFix(ip[1].vPosition, ip[2].vPosition, sideLen);

		// Deal with neighbours that are larger than us.
		if (ip[0].adjacency.neighbourMinusX > 1 && patchXY.x == 0)
			Output.Edges[0] = LargerNeighbourAdjacencyFix(ip[0].vPosition, ip[1].vPosition, patchXY.y, sideLen);
		if (ip[0].adjacency.neighbourMinusY > 1 && patchXY.y == 0)
			Output.Edges[1] = LargerNeighbourAdjacencyFix(ip[0].vPosition, ip[3].vPosition, patchXY.x, sideLen);	// NB: irregular index pattern - it's correct.
		if (ip[0].adjacency.neighbourPlusX > 1 && patchXY.x == PATCHES_PER_TILE_EDGE - 1)
			Output.Edges[2] = LargerNeighbourAdjacencyFix(ip[3].vPosition, ip[2].vPosition, patchXY.y, sideLen);
		if (ip[0].adjacency.neighbourPlusY > 1 && patchXY.y == PATCHES_PER_TILE_EDGE - 1)
			Output.Edges[3] = LargerNeighbourAdjacencyFix(ip[1].vPosition, ip[2].vPosition, patchXY.x, sideLen);	// NB: irregular index pattern - it's correct.

		// Use average of edge points for interior - visually looks OK.  
		// fxc bug if we assign different complex expressions to Inside[0] and [1].
		Output.Inside[1] = 0.5 * (Output.Edges[0] + Output.Edges[2]);
		Output.Inside[0] = 0.5 * (Output.Edges[1] + Output.Edges[3]);

		Output.vWorldXZ[0] = ip[0].vWorldXZ;
		Output.vWorldXZ[1] = ip[1].vWorldXZ;
		Output.vWorldXZ[2] = ip[2].vWorldXZ;
		Output.vWorldXZ[3] = ip[3].vWorldXZ;

		Output.debugColour[0] = DEBUG_COLOURS[clamp(log2(Output.Edges[0]), 0, 5)];
		Output.debugColour[1] = DEBUG_COLOURS[clamp(log2(Output.Edges[1]), 0, 5)];
		Output.debugColour[2] = DEBUG_COLOURS[clamp(log2(Output.Edges[2]), 0, 5)];
		Output.debugColour[3] = DEBUG_COLOURS[clamp(log2(Output.Edges[3]), 0, 5)];
		Output.debugColour[4] = DEBUG_COLOURS[clamp(log2(Output.Inside[0]), 0, 5)];
	}

	return Output;
}

// The hull shader is called once per output control point, which is specified with
// outputcontrolpoints.

// The input to the hull shader comes from the vertex shader

// The output from the hull shader will go to the domain shader.
// The tessellation factor, topology, and partition mode will go to the fixed function
// tessellator stage to calculate the UVW and domain points.
[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("TerrainScreenspaceLODConstantsHS")]
HS_OUTPUT TerrainScreenspaceLODHS(InputPatch<VS_CONTROL_POINT_OUTPUT, 4> p, uint i : SV_OutputControlPointID)
{
	// The VS displaces y for LOD calculations.  We drop it here so as not to displace twice in the DS.
	HS_OUTPUT Output;
	Output.vPosition = float3(p[i].vPosition.x, 0, p[i].vPosition.z);
	return Output;
}

