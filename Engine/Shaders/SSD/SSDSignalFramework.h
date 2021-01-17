#ifndef SSDSignalFramework_h
#define SSDSignalFramework_h 1

#include "SSD/SSDDefinitions.h"
#include "SSD/SSDSignalCore.h"

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

#endif