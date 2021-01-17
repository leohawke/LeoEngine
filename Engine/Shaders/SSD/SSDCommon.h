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
float2 BufferUVToOutputPixelPosition;
uint StateFrameIndexMod8;

uint2 BufferUVToBufferPixelCoord(float2 SceneBufferUV)
{
	return uint2(SceneBufferUV * BufferUVToOutputPixelPosition);
}

#endif