(RayTracing
	(RaytracingAccelerationStructure TLAS (space RAY_TRACING_REGISTER_SPACE_GLOBAL))
	(RWTexture2D Output (space RAY_TRACING_REGISTER_SPACE_GLOBAL))
	(texture2D Depth (space RAY_TRACING_REGISTER_SPACE_GLOBAL))
	(cbuffer GenShaderConstants (space RAY_TRACING_REGISTER_SPACE_GLOBAL)
		(float3 LightDirection)
	)
	(shader 
"
[shader("raygeneration")]
void RayGen()
{
		
}
"
	)
	(raygen_shader RayGen)
)