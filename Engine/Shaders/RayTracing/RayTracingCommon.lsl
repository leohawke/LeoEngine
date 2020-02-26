(RayTracing
(shader
"
#define RAY_TRACING_ENTRY_RAYGEN(name) [shader(\"raygeneration\")] void name()

#define RAY_TRACING_ENTRY_CLOSEST_HIT(name, payload_type, payload_name, attributes_type, attributes_name)\\
[shader(\"closesthit\")] void name(inout payload_type payload_name, in attributes_type attributes_name)

#define RAY_TRACING_ENTRY_ANY_HIT(name, payload_type, payload_name, attributes_type, attributes_name)\\
[shader(\"anyhit\")] void name(inout payload_type payload_name, in attributes_type attributes_name)

#define RAY_TRACING_ENTRY_MISS(name, payload_type, payload_name)\\
[shader(\"miss\")] void name(inout payload_type payload_name)

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

"
)
)
