(RayTracing
	(refer RayTracing/RayTracingCommon.lsl)
(shader
"
RAY_TRACING_ENTRY_CLOSEST_HIT(DefaultCHS,
	FMinimalPayload, Payload,
	FDefaultAttributes, Attributes)
{
	Payload.HitT = RayTCurrent();
}

RAY_TRACING_ENTRY_ANY_HIT(DefaultAHS,
	FMinimalPayload, Payload,
	FDefaultAttributes, Attributes)
{
	IgnoreHit();
}

"
)
(rayhitgroup_shader "closesthit=DefaultCHS anyhit=DefaultAHS")
)