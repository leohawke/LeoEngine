(RayTracing
	(refer RayTracingCommon.lsl)
(shader
"
RAY_TRACING_ENTRY_CLOSEST_HIT(DefaultCHS,
	FMinimalPayload, Payload,
	FDefaultAttributes, Attributes)
{
	Payload.HitT = RayTCurrent();
}

RAY_TRACING_ENTRY_MISS(DefaultMS,
	FMinimalPayload, Payload)
{
	Payload.SetMiss();
}
"
)
)