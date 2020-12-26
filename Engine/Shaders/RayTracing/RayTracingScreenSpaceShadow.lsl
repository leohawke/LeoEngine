(RayTracing
	(refer RayTracing/RayTracingCommon.lsl)
	(refer RayTracing/RayTracingDirectionalLight.lsl)
	(refer RandomSequence.lsl)
	(RaytracingAccelerationStructure (space RAY_TRACING_REGISTER_SPACE_GLOBAL) TLAS)
	(RWTexture2D  (elemtype float4) (space RAY_TRACING_REGISTER_SPACE_GLOBAL) Output)
	(texture2D (space RAY_TRACING_REGISTER_SPACE_GLOBAL) Depth)
	(cbuffer GenShaderConstants (space RAY_TRACING_REGISTER_SPACE_GLOBAL)
		(float3 LightDirection)
		(float SourceRadius)
		(uint SamplesPerPixel)
		(uint StateFrameIndex)
		(float4x4 CameraToWorld)
		(float2 Resolution)
	)
	(shader 
"
static const float FLT_MAX = asfloat(0x7F7FFFFF);

static const float DENOISER_INVALID_HIT_DISTANCE = -2.0;
static const float DENOISER_MISS_HIT_DISTANCE = -1.0;

uint CalcLinearIndex(uint2 PixelCoord)
{
	return PixelCoord.y * uint(Resolution.x) + PixelCoord.x;
}

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
		
	uint2 DTid = DispatchRaysIndex().xy;
	float2 xy = DTid.xy + 0.5;

	float2 readGBufferAt = xy;

	// Read depth and normal
	float sceneDepth = Depth.Load(int3(readGBufferAt, 0));

	 // Screen position for the ray
    float2 screenPos = xy / Resolution * 2.0 - 1.0;

	// Invert Y for DirectX-style coordinates
    screenPos.y = -screenPos.y;

	// Unproject into the world position using depth
	float4 unprojected = mul(float4(screenPos, sceneDepth, 1),CameraToWorld);
	float3 world = unprojected.xyz / unprojected.w;

	uint2 PixelCoord = DispatchRaysIndex().xy;

	RandomSequence RandSequence;
	uint LinearIndex = CalcLinearIndex(PixelCoord);
	RandomSequence_Initialize(RandSequence, LinearIndex, StateFrameIndex);

	OcclusionResult Out = InitOcclusionResult();
	for(uint SampleIndex =0;SampleIndex <SamplesPerPixel;++SampleIndex)
	{
		uint DummyVariable;
		float2 RandSample = RandomSequence_GenerateSample2D(RandSequence, DummyVariable);

		float3 RayOrigin;
		float3 RayDirection;
		float RayTMin;
		float RayTMax;

		LightShaderParameters LightParameters = {LightDirection,SourceRadius};

		GenerateDirectionalLightOcclusionRay(
			LightParameters,
			world,
			float3(0,1,0),
			RandSample,
			RayOrigin,
			RayDirection,
			RayTMin,
			RayTMax
		);

		RayDesc rayDesc = { RayOrigin,
			RayTMin,
			RayDirection,
			RayTMax };

		uint RayFlags = 0;

		RayFlags |= RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

		RayFlags |= RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;

		FMinimalPayload payload = {-1};

		TraceRay(TLAS, RayFlags, ~0,0,1,0, rayDesc, payload);

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