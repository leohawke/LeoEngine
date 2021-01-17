#ifndef SSDCommon_h
#define SSDCommon_h 1

#include "Random.h"
#include "SSD/SSDDefinitions.h"
#include "SSD/SSDMetadata.h"

float2 ViewportMin;
float2 ViewportMax;
float4 ThreadIdToBufferUV;
float4 BufferBilinearUVMinMax;
float4 BufferSizeAndInvSize;
float4 BufferUVToScreenPosition;
float2 BufferUVToOutputPixelPosition;
uint StateFrameIndexMod8;

uint2 BufferUVToBufferPixelCoord(float2 SceneBufferUV)
{
	return uint2(SceneBufferUV * BufferUVToOutputPixelPosition);
}

float2 DenoiserBufferUVToScreenPosition(float2 SceneBufferUV)
{
	return SceneBufferUV * BufferUVToScreenPosition.xy + BufferUVToScreenPosition.zw;
}


#if CONFIG_SIGNAL_PROCESSING == SIGNAL_PROCESSING_AO || CONFIG_SIGNAL_PROCESSING == SIGNAL_PROCESSING_SHADOW_VISIBILITY_MASK
// A gray scale valued encoded into a 16bit float only have 10bits mantissa.
#define TARGETED_SAMPLE_COUNT 1024
#endif

#ifndef CONFIG_METADATA_BUFFER_LAYOUT
#define CONFIG_METADATA_BUFFER_LAYOUT METADATA_BUFFER_LAYOUT_DISABLED
#endif

#endif