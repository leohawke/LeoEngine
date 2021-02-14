#ifndef RayTracingCommon_h
#define RayTracingCommon_h 1

#define RAY_TRACING_ENTRY_RAYGEN(name) [shader("raygeneration")] void name()

#define RAY_TRACING_ENTRY_CLOSEST_HIT(name, payload_type, payload_name, attributes_type, attributes_name)\
[shader("closesthit")] void name(inout payload_type payload_name, in attributes_type attributes_name)

#define RAY_TRACING_ENTRY_ANY_HIT(name, payload_type, payload_name, attributes_type, attributes_name)\
[shader("anyhit")] void name(inout payload_type payload_name, in attributes_type attributes_name)

#define RAY_TRACING_ENTRY_MISS(name, payload_type, payload_name)\
[shader("miss")] void name(inout payload_type payload_name)

struct FMinimalPayload
{
	float HitT; // Distance from ray origin to the intersection point in the ray direction. Negative on miss.

	bool IsMiss() { return HitT < 0; }
	bool IsHit() { return !IsMiss(); }

	void SetMiss() { HitT = -1; }
};

struct FDefaultAttributes
{
	float2 Barycentrics;
};

float4x4 SVPositionToWorld;
float3 WorldCameraOrigin;
float4 BufferSizeAndInvSize;


float3 ReconstructWorldPositionFromDeviceZ(uint2 PixelCoord, float DeviceZ)
{
	float4 WorldPosition = mul(float4(PixelCoord+0.5, DeviceZ, 1), SVPositionToWorld);
	WorldPosition.xyz /= WorldPosition.w;
	return WorldPosition.xyz;
}

uint CalcLinearIndex(uint2 PixelCoord)
{
	return PixelCoord.y * uint(BufferSizeAndInvSize.x) + PixelCoord.x;
}

// WorldNormal is the vector towards which the ray position will be offseted.
void ApplyPositionBias(inout RayDesc Ray, const float3 WorldNormal, const float MaxNormalBias)
{
	// Apply normal perturbation when defining ray to:
	// * avoid self intersection with current underlying triangle
	// * hide mismatch between shading surface & geometric surface
	//
	// While using shading normal is not correct (we should use the 
	// geometry normal, but it is not available atm/too costly to compute), 
	// it is good enough for a cheap solution
	const float MinBias = 0.01f;
	const float MaxBias = max(MinBias, MaxNormalBias);
	const float NormalBias = lerp(MaxBias, MinBias, saturate(dot(WorldNormal, Ray.Direction)));

	Ray.Origin += WorldNormal * NormalBias;
}

void ApplyCameraRelativeDepthBias(inout RayDesc Ray, uint2 PixelCoord, float DeviceZ, const float3 WorldNormal, const float AbsoluteNormalBias)
{
	float3 WorldPosition = ReconstructWorldPositionFromDeviceZ(PixelCoord, DeviceZ);
	float3 CameraDirection = WorldPosition - WorldCameraOrigin;
	float DistanceToCamera = length(CameraDirection);
	CameraDirection = normalize(CameraDirection);
	float Epsilon = 1.0e-4;
	float RelativeBias = DistanceToCamera * Epsilon;
	//float ProjectedBias = RelativeBias / dot(Ray.Direction, WorldNormal);

	float RayBias = max(RelativeBias, AbsoluteNormalBias);
	Ray.Origin -= CameraDirection * RayBias;
	ApplyPositionBias(Ray, WorldNormal, RayBias);
}

#endif
