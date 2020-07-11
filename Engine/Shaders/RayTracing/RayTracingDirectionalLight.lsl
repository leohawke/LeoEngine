(effect
(refer RayTracing/RayTracingCommon.lsl)
(refer MonteCarlo.lsl)
(shader
"

struct LightShaderParameters
{
	float3 Direction;
	float  SourceRadius;
};

// Adapted from \"A Low Distortion Map Between Disk and Square.\" Improved based
// on comments from Peter Shirley's blog post: \"Improved code for concentric map.\"
float2 ToConcentricMap(float2 RectangularCoords)
{
	float R;
	float Phi;

	RectangularCoords = 2.0 * RectangularCoords - 1.0;
	float2 RectangularCoordsSquared = RectangularCoords * RectangularCoords;
	if (RectangularCoordsSquared.x > RectangularCoordsSquared.y)
	{
		R = RectangularCoords.x;
		Phi = (PI / 4.0) * (RectangularCoords.y / RectangularCoords.x);
	}
	else
	{
		R = RectangularCoords.y;
		Phi = (PI / 2.0) - (PI / 4.0) * (RectangularCoords.x / RectangularCoords.y);
	}

	float2 PolarCoords = float2(R, Phi);
	return PolarCoords;
}

void GenerateDirectionalLightOcclusionRay(
	LightShaderParameters LightParameters,
	float3 WorldPosition,
	float3 WorldNormal,
	float2 RandSample,
	out float3 RayOrigin,
	out float3 RayDirection,
	out float RayTMin,
	out float RayTMax)
{
	// Draw random variable and choose a point on a unit disk
	float2 DiskUV = UniformSampleDiskConcentric(RandSample) * LightParameters.SourceRadius;

	// Permute light direction by user-defined radius on unit sphere
	float3 LightDirection = LightParameters.Direction;
	float3 N = LightDirection;
	float3 dPdu = float3(1, 0, 0);
	if (dot(N, dPdu) != 0)
	{
		dPdu = cross(N, dPdu);
	}
	else
	{
		dPdu = cross(N, float3(0, 1, 0));
	}
	float3 dPdv = cross(dPdu, N);
	LightDirection += dPdu * DiskUV.x + dPdv * DiskUV.y;
	
    RayOrigin = WorldPosition;
    RayDirection = normalize(LightDirection);
	RayTMin = 0.0;
	RayTMax = 1.0e27;
}
"
)
)

