(RayTracing
	(RaytracingAccelerationStructure (space RAY_TRACING_REGISTER_SPACE_GLOBAL) TLAS)
	(RWTexture2D (space RAY_TRACING_REGISTER_SPACE_GLOBAL) Output)
	(texture2D (space RAY_TRACING_REGISTER_SPACE_GLOBAL) Depth)
	(cbuffer GenShaderConstants (space RAY_TRACING_REGISTER_SPACE_GLOBAL)
		(float3 LightDirection)
	)
	(shader 
"
[shader(\"raygeneration\")]
void RayGen()
{
		
}
"
	)
	(raygen_shader RayGen)
)