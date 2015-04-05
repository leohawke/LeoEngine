
Texture2D normal :register(t0);//w:depth
Texture2D diffuse :register(t1);
Texture2D RandomVecMap :register(t2);

cbuffer SSAO : register(b0)
{
	float4x4 gProj;
	float4 gOffsetVectors[14];

	float    gOcclusionRadius = 0.5f;
	float    gOcclusionFadeStart = 0.2f;
	float    gOcclusionFadeEnd = 2.0f;
	float    gSurfaceEpsilon = 0.05f;
}

SamplerState LinearRepeat :register(s0);
SamplerState samNormalDepth : register(s1);
/*
Filter = MIN_MAG_LINEAR_MIP_POINT;

// Set a very far depth value if sampling outside of the NormalDepth map
// so we do not get false occlusions.
AddressU = BORDER;
AddressV = BORDER;
BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f);
*/

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane : TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

// Determines how much the sample point q occludes the point p as a function
// of distZ.
float OcclusionFunction(float distZ)
{
	//
	// If depth(q) is "behind" depth(p), then q cannot occlude p.  Moreover, if 
	// depth(q) and depth(p) are sufficiently close, then we also assume q cannot
	// occlude p because q needs to be in front of p by Epsilon to occlude p.
	//
	// We use the following function to determine the occlusion.  
	// 
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \ 
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//

	float occlusion = 0.0f;
	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;

		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}

	return occlusion;
}

#define gSampleCount 14
float main(VertexOut pin) : SV_TARGET{
	float4 NormalDepth = normal.SampleLevel(samNormalDepth,pin.Tex,0);
	float3 Diffuse = diffuse.Sample(LinearRepeat, pin.Tex).rgb;
	float ambient = 1.f;

	//SSAO
	float pz = NormalDepth.w;
	float3 p = (pz/pin.ToFarPlane.z)*pin.ToFarPlane;

	//[-1,1] -> [1,1]
	float3 randVec = 2.f* RandomVecMap.SampleLevel(LinearRepeat, 4.f*pin.Tex, 0.f).rgb - 1.f;
	float3 n = NormalDepth.xyz;
	//此光照是错误的,先算SSAO

	float occlusionSum = 0.f;
	[unroll]
	for (int i = 0; i < gSampleCount; ++i)
	{
		// Are offset vectors are fixed and uniformly distributed (so that our offset vectors
		// do not clump in the same direction).  If we reflect them about a random vector
		// then we get a random uniform distribution of offset vectors.
		float3 offset = reflect(gOffsetVectors[i].xyz, randVec);

		// Flip offset vector if it is behind the plane defined by (p, n).
		float flip = sign(dot(offset, n));

		// Sample a point near p within the occlusion radius.
		float3 q = p + flip * gOcclusionRadius * offset;

		// Project q and generate projective tex-coords.  
		// note [-1,-1] -> [1,1]
		float4 projQ = mul(float4(q, 1.0f), gProj);
		projQ /= projQ.w;

		// Find the nearest depth value along the ray from the eye to q (this is not
		// the depth of q, as q is just an arbitrary point near p and might
		// occupy empty space).  To find the nearest depth we look it up in the depthmap.

		float rz = normal.SampleLevel(samNormalDepth, projQ.xy, 0.0f).a;

		// Reconstruct full view space position r = (rx,ry,rz).  We know r
		// lies on the ray of q, so there exists a t such that r = t*q.
		// r.z = t*q.z ==> t = r.z / q.z
		float3 r;
		r = (rz / q.z) * q;

		//
		// Test whether r occludes p.
		//   * The product dot(n, normalize(r - p)) measures how much in front
		//     of the plane(p,n) the occluder point r is.  The more in front it is, the
		//     more occlusion weight we give it.  This also prevents self shadowing where 
		//     a point r on an angled plane (p,n) could give a false occlusion since they
		//     have different depth values with respect to the eye.
		//   * The weight of the occlusion is scaled based on how far the occluder is from
		//     the point we are computing the occlusion of.  If the occluder r is far away
		//     from p, then it does not occlude it.
		// 

		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);
		float occlusion = dp * OcclusionFunction(distZ);

		occlusionSum += occlusion;
	}

	occlusionSum /= gSampleCount;

	float access = 1.0f - occlusionSum;

	
	return access;
}