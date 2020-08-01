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

uint CalcLinearIndex(uint2 PixelCoord)
{
	return PixelCoord.y * uint(Resolution.x) + PixelCoord.x;
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

	float RayCount = 0;
	float Visibility =0;
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

		FMinimalPayload payload = {-1};

		TraceRay(TLAS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0,0,1,0, rayDesc, payload);

		RayCount += 1.0;
		Visibility += payload.IsMiss() ? 1.0:0.0;
	}
	Output[DispatchRaysIndex().xy].x = Visibility/RayCount;
}
"
	)
	(raygen_shader RayGen)
)