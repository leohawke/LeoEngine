#ifndef SSDSpatialKernel_h
#define SSDSpatialKernel_h 1

#include "SSD/SSDSignalAccumulatorArray.h"
#include "SSD/SSDSignalBufferEncoding.h"

/** Enums to choose how to compute the world distance for bilateral rejection. */
	// Only depends on the reference sample's pixel size and depth.
	#define SIGNAL_WORLD_FREQUENCY_REF_METADATA_ONLY 0

	// Only depends on the sample's pixel size and depth.
	#define SIGNAL_WORLD_FREQUENCY_SAMPLE_METADATA_ONLY 1

	// Is the smallest according of pixel size and depth between reference and sample.
	#define SIGNAL_WORLD_FREQUENCY_MIN_METADATA 2

	// Depends only based of the sample's hit distance and metadata.
	#define SIGNAL_WORLD_FREQUENCY_HIT_DISTANCE 3

	// Uses FSSDSignalSample::WorldBluringRadius precomputed in the sample.
	#define SIGNAL_WORLD_FREQUENCY_PRECOMPUTED_BLURING_RADIUS 4

	// Compute based on the harmonic being processed.
	#define SIGNAL_WORLD_FREQUENCY_HARMONIC 5

#ifndef CONFIG_VGPR_FREE_SAMPLE_TRACK_ID
#define CONFIG_VGPR_FREE_SAMPLE_TRACK_ID 0
#endif


#ifndef COMPILE_STACKOWIAK_KERNEL
#define COMPILE_STACKOWIAK_KERNEL 0
#endif

/** Configures the spatial kernel. */
struct FSSDKernelConfig
{
	// --------------------------- compile time.

	// Compile time set of sample to use.
	uint SampleSet;

	// Compile time selection of sample to use.
	uint SampleSubSetId;

	// Compile time layout of the buffer to accumulate.
	uint BufferLayout;

	// Compile time number of multiplexed signal per signal domain.
	uint MultiplexedSignalsPerSignalDomain;

	// Selects how the world distance should be computed for bilateral rejection at compile time.
	uint BilateralDistanceComputation;

	// Number of ring for a disk kernel.
	uint RingCount;

	/** Selects how the computation of world vector between the reference and neighbor should be computed. */
	uint NeighborToRefComputation;

	// Layout of RefSceneMetadata
	uint RefSceneMetadataLayout;

	// Multiplier applied on the world bluring distance of the signal.
	float WorldBluringDistanceMultiplier;

	// Compile time configuration whether want do LOOP or UNROLL
	//  false by default to expose in user code when the shader byte code might potentially be big.
	bool bUnroll;

	// Compile time whether the center of the kernel sample is sampled.
	bool bSampleKernelCenter;

	// Compile time whether sampling previous frame or current frame metadata.
	bool bPreviousFrameMetadata;

	// The sample should be accumulated starting from the further away.
	bool bDescOrder;

	// Whether a sample should be normalised to 1 before accmulation.
	bool bNormalizeSample;

	// Whether should min sample frequency of pair of samples
	// [ Jimenez 2014, "Next Generation Post Processing in Call of Duty: Advanced Warfare" ]
	bool bMinSamplePairInvFrequency;

	// Whether the bilateral distance should be maxed with reference bilateral distance.
	bool bMaxWithRefBilateralDistance;

	// Whether the spherical harmonic of a sample should be computed before accumulation.
	bool bComputeSampleColorSH;

	// Whether should clamp the UV individually per signal.
	bool bClampUVPerMultiplexedSignal;

	// The color space that has been encoded in the buffer.
	uint BufferColorSpace[SIGNAL_ARRAY_SIZE];

	// The color space of the accumulation.
	uint AccumulatorColorSpace[SIGNAL_ARRAY_SIZE];

	// The color space of the accumulation.
	uint BilateralSettings[SIGNAL_ARRAY_SIZE];


	// --------------------------- Per wave.

	// Buffer size and inv size.
	float4 BufferSizeAndInvSize;
	float4 BufferBilinearUVMinMax;

	// Mip level in the buffer to sample.
	float BufferMipLevel;

	// Multiplier on the sample's offset.
	float KernelSpreadFactor;

	// The periode of the harmonic being sampled.
	float HarmonicPeriode;

	// Buffer's min and max UV, per texture.
	float4 PerSignalUVMinMax[SIGNAL_ARRAY_SIZE];


	// --------------------------- Per lane.

	// Number of samples should be done when doing variable box sampling.
	uint BoxKernelRadius;

	// Runtime number of samples
	uint SampleCount;

	// Buffer coordinate of the center of the kernel.
	float2 BufferUV;

	// Metadata of the scene for the bilateral therm.
	FSSDCompressedSceneInfos CompressedRefSceneMetadata;

	// Buffer coordinate of the reference used for decompression.
	// Please try to make this same as BufferUV.
	float2 RefBufferUV;

	// Runtime to force the first sample of the kernel to be accumulated.
	bool bForceKernelCenterAccumulation;

	// Runtime to force accumulating all sample.
	bool bForceAllAccumulation;

	// Runtime selection of a track of sample.
	uint SampleTrackId;

	// Reference meta data.
	float RefBilateralDistance[SIGNAL_ARRAY_SIZE];

	// Uniform random values required for stocastic kernel.
	float Randoms[1];

	// Seed for hamerley sequence used for stocastic kernel.
	uint2 HammersleySeed;

	// Normalized pixel space direction for directional kernels.
	float2 MajorAxis;

	// The pixel radius along the major and minor axes for directional kernels.
	float MajorPixelRadius;
	float MinorPixelRadius;
};

FSSDKernelConfig CreateKernelConfig()
{
	FSSDKernelConfig KernelConfig;
	KernelConfig.SampleSet = SAMPLE_SET_1X1;
	KernelConfig.SampleSubSetId = 0;
	KernelConfig.BufferLayout = SIGNAL_BUFFER_LAYOUT_UNINITIALIZED;
	KernelConfig.MultiplexedSignalsPerSignalDomain = SIGNAL_ARRAY_SIZE;
	KernelConfig.NeighborToRefComputation = NEIGHBOR_TO_REF_CACHE_WORLD_POSITION;
	KernelConfig.RefSceneMetadataLayout = METADATA_BUFFER_LAYOUT_DISABLED;
	KernelConfig.RingCount = 0;
	KernelConfig.WorldBluringDistanceMultiplier = 1.0;
	KernelConfig.bUnroll = false;
	KernelConfig.bSampleKernelCenter = false;
	KernelConfig.bPreviousFrameMetadata = false;
	KernelConfig.BilateralDistanceComputation = SIGNAL_WORLD_FREQUENCY_MIN_METADATA;
	KernelConfig.bDescOrder = false;
	KernelConfig.bNormalizeSample = false;
	KernelConfig.bMinSamplePairInvFrequency = false;
	KernelConfig.bMaxWithRefBilateralDistance = false;
	KernelConfig.bComputeSampleColorSH = false;
	KernelConfig.bClampUVPerMultiplexedSignal = false;

	{
		[unroll(SIGNAL_ARRAY_SIZE)]
			for (uint MultiplexId = 0; MultiplexId < SIGNAL_ARRAY_SIZE; MultiplexId++)
			{
				KernelConfig.BufferColorSpace[MultiplexId] = STANDARD_BUFFER_COLOR_SPACE;
				KernelConfig.AccumulatorColorSpace[MultiplexId] = STANDARD_BUFFER_COLOR_SPACE;
				KernelConfig.BilateralSettings[MultiplexId] = 0x0000;
			}
	}

	// SGPRs.
	KernelConfig.BufferSizeAndInvSize = float4(0, 0, 0, 0);
	KernelConfig.BufferBilinearUVMinMax = float4(0, 0, 0, 0);
	KernelConfig.BufferMipLevel = 0.0;
	KernelConfig.KernelSpreadFactor = 1;
	KernelConfig.HarmonicPeriode = 1.0;

	{
		[unroll(SIGNAL_ARRAY_SIZE)]
			for (uint MultiplexId = 0; MultiplexId < SIGNAL_ARRAY_SIZE; MultiplexId++)
			{
				KernelConfig.PerSignalUVMinMax[MultiplexId] = 0.0;
			}
	}

	// VGPRs.
	KernelConfig.BoxKernelRadius = 1;
	KernelConfig.SampleCount = 1;
	KernelConfig.BufferUV = 0.0;
	KernelConfig.CompressedRefSceneMetadata = CreateCompressedSceneInfos();
	KernelConfig.RefBufferUV = 0.0;
	KernelConfig.bForceKernelCenterAccumulation = false;
	KernelConfig.bForceAllAccumulation = false;
	KernelConfig.SampleTrackId = 0;
	KernelConfig.MajorAxis = 0.0;
	KernelConfig.MajorPixelRadius = 0.0;
	KernelConfig.MinorPixelRadius = 0.0;
	KernelConfig.HammersleySeed = 0;

	{
		[unroll(SIGNAL_ARRAY_SIZE)]
		for (uint MultiplexId = 0; MultiplexId < SIGNAL_ARRAY_SIZE; MultiplexId++)
		{
			KernelConfig.RefBilateralDistance[MultiplexId] = 0.0;
		}
	}

	{
		[unroll(SIGNAL_ARRAY_SIZE)]
		for (uint RandomSignalId = 0; RandomSignalId < 1; RandomSignalId++)
		{
			KernelConfig.Randoms[RandomSignalId] = 0.0;
		}
	}

	return KernelConfig;
}

void SetBilateralPreset(uint BilateralPresetId, inout FSSDKernelConfig KernelConfig)
{
	if (BilateralPresetId == BILATERAL_PRESET_MONOCHROMATIC_PENUMBRA)
	{
		[unroll(SIGNAL_ARRAY_SIZE)]
			for (uint MultiplexId = 0; MultiplexId < SIGNAL_ARRAY_SIZE; MultiplexId++)
			{
				// Shadow masks are normal invarient, so only reject based on position.
				KernelConfig.BilateralSettings[MultiplexId] = BILATERAL_POSITION_BASED(5);
			}
	}
}

/** Compute at compile time the index of the signal in the batch, from the index of the multiplexed signal. */
uint ComputeSignalBatchIdFromSignalMultiplexId(FSSDKernelConfig KernelConfig, const uint SignalMultiplexId)
{
	return SignalMultiplexId / KernelConfig.MultiplexedSignalsPerSignalDomain;
}

#endif