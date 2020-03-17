(RayTracing
	(refer RayTracingCommon.lsl)
	(RaytracingAccelerationStructure (space RAY_TRACING_REGISTER_SPACE_GLOBAL) TLAS)
	(RWTexture2D  (elemtype float) (space RAY_TRACING_REGISTER_SPACE_GLOBAL) Output)
	(texture2D (space RAY_TRACING_REGISTER_SPACE_GLOBAL) Depth)
	(cbuffer GenShaderConstants (space RAY_TRACING_REGISTER_SPACE_GLOBAL)
		(float3 LightDirection)
		(float4x4 CameraToWorld)
		(float2 Resolution)
	)
	(shader 
"
static const float FLT_MAX = asfloat(0x7F7FFFFF);

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

	// R
    float3 direction = LightDirection;
    float3 origin = world;

	RayDesc rayDesc = { origin,
        0.1f,
        direction,
        FLT_MAX };

	FMinimalPayload payload = {-1};

	TraceRay(TLAS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0,0,1,0, rayDesc, payload);

	if(payload.IsHit() && payload.HitT < FLT_MAX)
		Output[DispatchRaysIndex().xy] = 0;
	else
		Output[DispatchRaysIndex().xy] = 1;
}
"
	)
	(raygen_shader RayGen)
)