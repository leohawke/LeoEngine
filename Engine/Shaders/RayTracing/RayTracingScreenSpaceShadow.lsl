(RayTracing
	(include RayTracing/RayTracingCommon.h)
	(include RayTracing/RayTracingDirectionalLight.h)
	(include RandomSequence.h)
	(RaytracingAccelerationStructure (space RAY_TRACING_REGISTER_SPACE_GLOBAL) TLAS)
	(RWTexture2D  (elemtype float4) (space RAY_TRACING_REGISTER_SPACE_GLOBAL) Output)
	(shader 
"
Texture2D WorldNormalBuffer;
Texture2D Depth;
float NormalBias;
float3 LightDirection;
float SourceRadius;
uint SamplesPerPixel;
uint StateFrameIndex;

static const float FLT_MAX = asfloat(0x7F7FFFFF);

static const float DENOISER_INVALID_HIT_DISTANCE = -2.0;
static const float DENOISER_MISS_HIT_DISTANCE = -1.0;

struct OcclusionResult
{
	float Visibility;
	float HitCount;
	float ClosestRayDistance;
	float RayCount;
};

float OcclusionToShadow(OcclusionResult In, uint LocalSamplesPerPixel)
{
	return (LocalSamplesPerPixel > 0) ? In.Visibility / LocalSamplesPerPixel : In.Visibility;
}



OcclusionResult InitOcclusionResult()
{
	OcclusionResult Out;

	Out.Visibility = 0.0;
	Out.ClosestRayDistance = DENOISER_INVALID_HIT_DISTANCE;
	Out.HitCount = 0.0;
	Out.RayCount = 0.0;

	return Out;
}

RAY_TRACING_ENTRY_RAYGEN(RayGen)
{
	uint2 PixelCoord = DispatchRaysIndex().xy;
		
	// Read depth and normal
	float DeviceZ = Depth.Load(int3(PixelCoord, 0)).r;

	float3 WorldNormal = WorldNormalBuffer.Load(int3(PixelCoord, 0)).xyz;
	float3 WorldPosition = ReconstructWorldPositionFromDeviceZ(PixelCoord, DeviceZ);

	RandomSequence RandSequence;
	uint LinearIndex = CalcLinearIndex(PixelCoord);
	RandomSequence_Initialize(RandSequence, LinearIndex, StateFrameIndex);

	OcclusionResult Out = InitOcclusionResult();

	bool bApplyNormalCulling = true;

	for(uint SampleIndex =0;SampleIndex <SamplesPerPixel;++SampleIndex)
	{
		uint DummyVariable;
		float2 RandSample = RandomSequence_GenerateSample2D(RandSequence, DummyVariable);

		LightShaderParameters LightParameters = {LightDirection,SourceRadius};

		RayDesc Ray;
		GenerateDirectionalLightOcclusionRay(
			LightParameters,
			WorldPosition,
			float3(0,1,0),
			RandSample,
			Ray.Origin,
			Ray.Direction,
			Ray.TMin,
			Ray.TMax
		);
		ApplyCameraRelativeDepthBias(Ray, PixelCoord, DeviceZ, WorldNormal, NormalBias);

		if(bApplyNormalCulling && dot(WorldNormal, Ray.Direction) <= 0.0)
		{
			//self-intersection
			Out.ClosestRayDistance = 0.001;
			continue;
		}

		uint RayFlags = 0;

		RayFlags |= RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

		RayFlags |= RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;

		FMinimalPayload payload = {-1};

		TraceRay(TLAS, RayFlags, ~0,0,1,0, Ray, payload);

		Out.RayCount += 1.0;
		Out.Visibility += payload.IsMiss() ? 1.0:0.0;

		if(payload.IsHit())
		{
			Out.ClosestRayDistance =
				(Out.ClosestRayDistance == DENOISER_INVALID_HIT_DISTANCE) ||
				(payload.HitT < Out.ClosestRayDistance) ? payload.HitT : Out.ClosestRayDistance;
			Out.HitCount += 1.0;
		}
		else
		{
			Out.ClosestRayDistance = (Out.ClosestRayDistance == DENOISER_INVALID_HIT_DISTANCE) ? DENOISER_MISS_HIT_DISTANCE : Out.ClosestRayDistance;
		}
	}

	const float Shadow = OcclusionToShadow(Out, SamplesPerPixel);

	Output[DispatchRaysIndex().xy] =float4(Shadow,Out.ClosestRayDistance,0,0);
}
"
	)
	(raygen_shader RayGen)
)