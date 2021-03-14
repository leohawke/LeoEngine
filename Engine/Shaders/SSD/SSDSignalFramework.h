#ifndef SSDSignalFramework_h
#define SSDSignalFramework_h 1

#include "SSD/SSDDefinitions.h"
#include "SSD/SSDSignalCore.h"

#ifndef CONFIG_SPECULAR_RATIO_ESTIMATOR
#define CONFIG_SPECULAR_RATIO_ESTIMATOR 0
#endif

/** Color space to transform the color to. */
#ifndef CONFIG_COLOR_SPACE_TRANSFORMATION
#define CONFIG_COLOR_SPACE_TRANSFORMATION COLOR_SPACE_TRANSFORMATION_DISABLED
#endif

float HitDistanceToWorldBluringRadius;

/** Return whether this signal is occluded or not. */
#if CONFIG_SIGNAL_PROCESSING == SIGNAL_PROCESSING_SHADOW_VISIBILITY_MASK
bool IsMissSample(FSSDSignalSample Sample)
{
	return Sample.MissCount != 0.0;
}
#endif

/** Return whether this signal is valid or not. */
bool IsInvalidSample(FSSDSignalSample Sample)
{
	return Sample.SampleCount == 0.0;
}

/** COmpute the weight of the bilateral filter to use between a reference sample and a neighbor. */
float ComputeBilateralWeight(
	const uint BilateralSettings,
	float MaxWorldBluringDistance,
	FSSDSampleSceneInfos RefSceneMetadata,
	FSSDSampleSceneInfos NeighborSceneMetadata,
	float3 NeighborToRefVector)
{
#if CONFIG_USE_VIEW_SPACE
	float3 RefNormal = RefSceneMetadata.ViewNormal;
	float3 NeighborNormal = NeighborSceneMetadata.ViewNormal;
#else
	float3 RefNormal = RefSceneMetadata.WorldNormal;
	float3 NeighborNormal = NeighborSceneMetadata.WorldNormal;
#endif

	float PositionBilateralWeight = 1;
	/*if ((BilateralSettings & 0xF) == 1)
	{
		const float WorldRadius = MaxWorldBluringDistance;

		float RefAnisotropyInvFactor = ComputeAnisotropyInvFactor(RefSceneMetadata);

		float3 CameraToRef = View.ViewForward;

		float Z = dot(CameraToRef, NeighborToRefVector);
		float XY = length2(NeighborToRefVector - CameraToRef * Z * (1 - RefAnisotropyInvFactor));

		float DistSquare = XY;

		float Multiplier = rcp(WorldRadius * WorldRadius);

		PositionBilateralWeight = saturate(1 - DistSquare * Multiplier);
	}
	else */if ((BilateralSettings & 0xF) == 2)
	{
		// TODO(Denoiser): save 2 VGPR by using SceneDepth instead of RefSceneMetadata.TranslatedWorldPosition (not sure it can work in math yet).
		// float2 ClipPosition = ScreenPosition * (View.ViewToClip[3][3] < 1.0f ? WorldDepth : 1.0f);
		// Infos.TranslatedWorldPosition = mul(float4(ClipPosition, WorldDepth, 1), ScreenToTranslatedWorld).xyz;

		float DistFromRefPlane = abs(dot(RefNormal, NeighborToRefVector));

		PositionBilateralWeight = saturate(1 - DistFromRefPlane * rcp(MaxWorldBluringDistance));
	}
	else if ((BilateralSettings & 0xF) == 3)
	{
		if (dot(NeighborToRefVector, NeighborToRefVector) > MaxWorldBluringDistance * MaxWorldBluringDistance)
		{
			PositionBilateralWeight = 0;
		}
	}
	else if ((BilateralSettings & 0xF) == 4)
	{
		// TODO(Denoiser): extremely expensive with rcp()...
		PositionBilateralWeight = saturate(1 - dot(NeighborToRefVector, NeighborToRefVector) * rcp(MaxWorldBluringDistance * MaxWorldBluringDistance));
	}
	else if ((BilateralSettings & 0xF) == 5)
	{
		float DistFromRefPlane = dot(RefNormal, NeighborToRefVector);

		// TODO(Denoiser): extremely expensive with rcp()...
		PositionBilateralWeight = (
			saturate(1 - dot(NeighborToRefVector, NeighborToRefVector) * rcp(MaxWorldBluringDistance * MaxWorldBluringDistance)) *
			saturate(1 - 6 * abs(DistFromRefPlane) * rcp(MaxWorldBluringDistance)));
	}

	float NormalBilateralWeight = 1;
	if (BilateralSettings & BILATERAL_NORMAL)
	{
		float NoN = max(dot(RefNormal, NeighborNormal), 0);

#if 1 // TODO(Denoiser)
		NormalBilateralWeight = pow(NoN, 4);
#else
		// [ Kontkanen et al 2004, "Irradiance Filtering for Monte Carlo Ray Tracing" ]
		// TODO(Denoiser): looks broken
		float m = MaxWorldBluringDistance;
		float a = 0.3;
		float d = sqrt(1 - NoN); // + length(RefSceneMetadata.TranslatedWorldPosition - NeighborSceneMetadata.TranslatedWorldPosition) * rcp(m);
		float x = d * rcp(a);

		NormalBilateralWeight = exp(-x * x);
#endif
	}

	// [ Tokoyashi 2015, "Specular Lobe-Aware Filtering and Upsampling for Interactive Indirect Illumination" ]
	float LobeSimilarity = 1;
	float AxesSimilarity = 1;
	/*if (BilateralSettings & (BILATERAL_TOKOYASHI_LOBE | BILATERAL_TOKOYASHI_AXES))
	{
		const float Beta = 32;

		FSphericalGaussian Ref = ComputeRoughnessLobe(RefSceneMetadata);
		FSphericalGaussian Neighbor = ComputeRoughnessLobe(NeighborSceneMetadata);

		float InvSharpnessSum = rcp(Ref.Sharpness + Neighbor.Sharpness);

		if (BilateralSettings & BILATERAL_TOKOYASHI_LOBE)
			LobeSimilarity = pow(2 * sqrt(Ref.Sharpness * Neighbor.Sharpness) * InvSharpnessSum, Beta);

		if (BilateralSettings & BILATERAL_TOKOYASHI_AXES)
			AxesSimilarity = exp(-(Beta * (Ref.Sharpness * Neighbor.Sharpness) * InvSharpnessSum) * saturate(1 - dot(Ref.Axis, Neighbor.Axis)));
	}*/

	return PositionBilateralWeight * NormalBilateralWeight * LobeSimilarity * AxesSimilarity;
}

float GetSignalWorldBluringRadius(FSSDSignalSample Sample, FSSDSampleSceneInfos SceneMetadata, uint BatchedSignalId)
#if CONFIG_SIGNAL_PROCESSING == SIGNAL_PROCESSING_SHADOW_VISIBILITY_MASK
{
	if (IsInvalidSample(Sample))
	{
		return WORLD_RADIUS_INVALID;
	}
	else if (IsMissSample(Sample))
	{
		return WORLD_RADIUS_MISS;
	}

	return HitDistanceToWorldBluringRadius * Sample.ClosestHitDistance;
}
#endif


// Returns the penumbra of this sample, or 1 if invalid.
float GetSamplePenumbraSafe(FSSDSignalSample Sample)
{
	return (Sample.SampleCount > 0 ? Sample.MissCount / Sample.SampleCount : 1);
}

/** Conveniently transform one simple from a source basis to a destination basis. */
FSSDSignalSample TransformSignal(FSSDSignalSample Sample, const uint SrcBasis, const uint DestBasis)
#if COMPILE_SIGNAL_COLOR
{
	// If the source and destination bases are the same, early return to not pay ALU and floating point divergence.
	if (SrcBasis == DestBasis)
	{
		return Sample;
	}

	// TODO(Denoiser): should force this to be scalar load.
	float FrameExposureScale = GetSignalColorTransformationExposure();

	// TODO(Denoiser): exposes optimisation if sample is normalized.
	const bool bIsNormalizedSample = false;
	const bool bDebugForceNormalizeColor = true;
	const bool bIsNormalizedColor = bIsNormalizedSample || bDebugForceNormalizeColor;


	const bool bUnchangeAlphaPremultiply = (SrcBasis & COLOR_SPACE_UNPREMULTIPLY) == (DestBasis & COLOR_SPACE_UNPREMULTIPLY);
	const bool bUnchangeColorSpace = bUnchangeAlphaPremultiply && (SrcBasis & 0x3) == (DestBasis & 0x3);

	if (bDebugForceNormalizeColor)
	{
		Sample.SceneColor *= SafeRcp(Sample.SampleCount);
	}

	// Remove Karis weighting before doing any color transformations.
	if (SrcBasis & COLOR_SPACE_KARIS_WEIGHTING)
	{
		float KarisX = Luma4(Sample.SceneColor.rgb);
		if ((SrcBasis & 0x3))
		{
			KarisX = Sample.SceneColor.x;
		}

		if (!bIsNormalizedColor)
		{
			KarisX *= SafeRcp(Sample.SampleCount);
		}

		Sample.SceneColor *= KarisHdrWeightInvY(KarisX, FrameExposureScale);
	}

	if (bUnchangeColorSpace)
	{
		// NOP
	}
	else if ((SrcBasis & 0x3) == COLOR_SPACE_YCOCG)
	{
		Sample.SceneColor.rgb = YCoCg_2_LinearRGB(Sample.SceneColor.rgb);
	}
	else if ((SrcBasis & 0x3) == COLOR_SPACE_LCOCG)
	{
		Sample.SceneColor.rgb = LCoCg_2_LinearRGB(Sample.SceneColor.rgb);
	}

	float Alpha = Sample.SceneColor.a * SafeRcp(Sample.SampleCount);
	if (bIsNormalizedColor)
	{
		Alpha = Sample.SceneColor.a;
	}

	// Premultiplied RGBA
	if (bUnchangeAlphaPremultiply)
	{
		// NOP
	}
	else if (SrcBasis & COLOR_SPACE_UNPREMULTIPLY)
	{
		Sample.SceneColor.rgb *= Alpha;
	}
	else //if (DestBasis & COLOR_SPACE_UNPREMULTIPLY)
	{
		Sample.SceneColor.rgb *= SafeRcp(Alpha);
	}

	float x = Luma4(Sample.SceneColor.rgb);
	if ((DestBasis & 0x3) == COLOR_SPACE_YCOCG)
	{
		if (!bUnchangeColorSpace)
			Sample.SceneColor.xyz = LinearRGB_2_YCoCg(Sample.SceneColor.rgb);

		x = Sample.SceneColor.x;
	}
	else if ((DestBasis & 0x3) == COLOR_SPACE_LCOCG)
	{
		if (!bUnchangeColorSpace)
			Sample.SceneColor.xyz = LinearRGB_2_LCoCg(Sample.SceneColor.rgb);

		x = Sample.SceneColor.x;
	}

	if (DestBasis & COLOR_SPACE_KARIS_WEIGHTING)
	{
		if (bIsNormalizedColor)
		{
			Sample.SceneColor *= KarisHdrWeightY(x, FrameExposureScale);
		}
		else
		{
			Sample.SceneColor *= KarisHdrWeightY(x * SafeRcp(Sample.SampleCount), FrameExposureScale);
		}
	}

	if (bDebugForceNormalizeColor)
	{
		Sample.SceneColor *= Sample.SampleCount;
	}

	return Sample;
}
#else
{
	return Sample;
}
#endif

/* Get the weight for multi importance sampling.*/
float GetRatioEstimatorWeight(
	FSSDSampleSceneInfos RefSceneMetadata,
	FSSDSampleSceneInfos SampleSceneMetadata,
	FSSDSignalSample Sample,
	uint2 SamplePixelCoord)
#if CONFIG_SPECULAR_RATIO_ESTIMATOR
	// [ Stackowiak 2018, "Stochastic all the things: Raytracing in hybrid real-time rendering" ]
{
#if CONFIG_USE_VIEW_SPACE
#error Ratio estimator should be done in world space to ensure exact same ray direction.
#endif

	// If invalid ray or geometric miss, just.
	if (IsInvalidSample(Sample) && 0)
	{
		return 0;
	}

	float3 OriginalRayDirection;
	float OriginalRayPDF;

	GetInputRayDirectionPixel(
		SamplePixelCoord,
		SampleSceneMetadata,
		/* out */ OriginalRayDirection,
		/* out */ OriginalRayPDF);

	// min to avoid floating point underflow that causes artifact by D anormally high.
	float ClosestHitDistance = min(Sample.ClosestHitDistance, 1000000);

	float a2 = Pow4(RefSceneMetadata.Roughness);

	// TODO(Denoiser): Improve quantization in CONFIG_COMPRESS_WORLD_NORMAL and use it.
	float3 N = GetWorldNormal(RefSceneMetadata);

	float NoH;
#if CONFIG_VALU_OPTIMIZATIONS
	{
		float3 V = (View.TranslatedWorldCameraOrigin - GetTranslatedWorldPosition(RefSceneMetadata));
		float3 L = ((GetTranslatedWorldPosition(SampleSceneMetadata) - GetTranslatedWorldPosition(RefSceneMetadata)) + OriginalRayDirection * ClosestHitDistance);
		float3 H = (L + V * (sqrt(length2(L)) * rsqrt(length2(V))));

		NoH = saturate(dot(N, H) * rsqrt(length2(H)));
	}
#else
	{
		float3 V = normalize(View.TranslatedWorldCameraOrigin - GetTranslatedWorldPosition(RefSceneMetadata));
		float3 L = normalize((GetTranslatedWorldPosition(SampleSceneMetadata) - GetTranslatedWorldPosition(RefSceneMetadata)) + OriginalRayDirection * ClosestHitDistance);
		float3 H = normalize(L + V);

		NoH = abs(dot(N, H));
	}
#endif


#if 1
	{
		const float MaximumWeight = 10;

		float d = saturate(clamp(NoH * a2 - NoH, -NoH, 0) * NoH + 1);

		if (d == 0)
		{
			return MaximumWeight;
		}

		return clamp(a2 / (PI * d * d * OriginalRayPDF), 0, MaximumWeight);
	}
#else
	{
		float D = D_GGX(a2, NoH);

		return clamp(D / OriginalRayPDF, 0, 10);
	}
#endif
}
#else // !CONFIG_SPECULAR_RATIO_ESTIMATOR
{
	return 1.0;
}
#endif // CONFIG_SPECULAR_RATIO_ESTIMATOR

#endif