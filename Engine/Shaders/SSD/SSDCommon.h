#ifndef SSDCommon_h
#define SSDCommon_h 1

#include "Random.h"
#include "Common.h"
#include "SSD/SSDDefinitions.h"
#include "SSD/SSDMetadata.h"

#if CONFIG_SIGNAL_PROCESSING == SIGNAL_PROCESSING_AO || CONFIG_SIGNAL_PROCESSING == SIGNAL_PROCESSING_SHADOW_VISIBILITY_MASK
// A gray scale valued encoded into a 16bit float only have 10bits mantissa.
#define TARGETED_SAMPLE_COUNT 1024
#endif

#ifndef CONFIG_METADATA_BUFFER_LAYOUT
#define CONFIG_METADATA_BUFFER_LAYOUT METADATA_BUFFER_LAYOUT_DISABLED
#endif

#ifndef CONFIG_USE_VIEW_SPACE
#define CONFIG_USE_VIEW_SPACE 0
#endif


uint2 ViewportMin;
uint2 ViewportMax;
float4 ThreadIdToBufferUV;
float4 BufferBilinearUVMinMax;
float4 BufferSizeAndInvSize;
float4 BufferUVToScreenPosition;
float2 BufferUVToOutputPixelPosition;
uint StateFrameIndexMod8;
float4x4 ScreenToTranslatedWorld;
float4x4 ViewToClip;
float4x4 TranslatedWorldToView;

//SceneTextureParameters
Texture2D SceneDepthBuffer;
SamplerState SceneDepthBufferSampler;
Texture2D WorldNormalBuffer;
SamplerState WorldNormalSampler;

float WorldDepthToPixelWorldRadius;

SamplerState GlobalPointClampedSampler;




/** Returns the radius of a pixel in world space. */
float ComputeWorldBluringRadiusCausedByPixelSize(FSSDSampleSceneInfos SceneMetadata)
{
	// Should be multiplied 0.5* for the diameter to radius, and by 2.0 because GetTanHalfFieldOfView() cover only half of the pixels.
	return WorldDepthToPixelWorldRadius * GetWorldDepth(SceneMetadata);
}

uint2 BufferUVToBufferPixelCoord(float2 SceneBufferUV)
{
	return uint2(SceneBufferUV * BufferUVToOutputPixelPosition);
}

float2 DenoiserBufferUVToScreenPosition(float2 SceneBufferUV)
{
	return SceneBufferUV * BufferUVToScreenPosition.xy + BufferUVToScreenPosition.zw;
}



float SafeRcp(float x, float d = 0.0)
{
	return x > 0.0 ? rcp(x) : d;
}

FSSDCompressedSceneInfos SampleCompressedSceneMetadata(
	const bool bPrevFrame,
	float2 BufferUV, uint2 PixelCoord)
#if CONFIG_METADATA_BUFFER_LAYOUT == METADATA_BUFFER_LAYOUT_DISABLED
{
	FSSDCompressedSceneInfos CompressedMetadata = CreateCompressedSceneInfos();

	if (bPrevFrame)
	{
	}
	else
	{
		float DeviceZ = SceneDepthBuffer.SampleLevel(SceneDepthBufferSampler, BufferUV, 0).x;
		CompressedMetadata.VGPR[0] = asuint(ConvertFromDeviceZ(DeviceZ));
		float3 WorldNormal = WorldNormalBuffer.SampleLevel(WorldNormalSampler, BufferUV, 0).xyz;
		CompressedMetadata.VGPR[1] = asuint(WorldNormal.x);
		CompressedMetadata.VGPR[2] = asuint(WorldNormal.y);
		CompressedMetadata.VGPR[3] = asuint(WorldNormal.z);
	}

	return CompressedMetadata;
}
#else
{
	FSSDCompressedSceneInfos CompressedMetadata = CreateCompressedSceneInfos();

	Texture2D<uint> CompressedMetadata0 = bPrevFrame ? PrevCompressedMetadata_0 : CompressedMetadata_0;

	int3 Coord = int3(PixelCoord, /* MipLevel = */ 0);
	CompressedMetadata.VGPR[0] = CompressedMetadata0.Load(Coord);

	return CompressedMetadata;
}
#endif

#endif