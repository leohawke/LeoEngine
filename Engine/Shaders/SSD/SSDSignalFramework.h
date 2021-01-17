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

#endif