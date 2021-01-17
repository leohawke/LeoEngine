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

#ifndef CONFIG_ACCUMULATOR_VGPR_COMPRESSION
#define CONFIG_ACCUMULATOR_VGPR_COMPRESSION ACCUMULATOR_COMPRESSION_DISABLED
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

FSSDSignalSample TransformSignalSampleForAccumulation(
	FSSDKernelConfig KernelConfig,
	uint MultiplexId,
	FSSDSampleSceneInfos SampleSceneMetadata,
	FSSDSignalSample Sample,
	uint2 SamplePixelCoord)
{
	// Transform the color space.
	// TODO(Denoiser): could pass down information that this sample may be normalized.
	Sample = TransformSignal(
		Sample,
		/* SrcBasis  = */ KernelConfig.BufferColorSpace[MultiplexId],
		/* DestBasis = */ KernelConfig.AccumulatorColorSpace[MultiplexId]);

	// Compute the spherical harmonic of the sample.
#if COMPILE_SIGNAL_COLOR_SH && COMPILE_SIGNAL_COLOR
	if (KernelConfig.bComputeSampleColorSH)
	{
		Sample.ColorSH = ComputeSampleColorSH(SampleSceneMetadata, Sample, SamplePixelCoord);
	}
#endif

	return Sample;
}

float2 ComputeRefBufferUV(FSSDKernelConfig KernelConfig)
{
	if (KernelConfig.bPreviousFrameMetadata)
	{
		// Impossible to compute from BufferUV because it's in the previous frame basis.
		return KernelConfig.RefBufferUV;
	}
	else if (KernelConfig.SampleSet == SAMPLE_SET_HEXAWEB)
	{
		// Impossible to compute from BufferUV because of random offset certainely needed using this..
		return KernelConfig.RefBufferUV;
	}
	else if (KernelConfig.SampleSet == SAMPLE_SET_STACKOWIAK_4_SETS)
	{
		uint SampleTrackId = KernelConfig.SampleTrackId;

#if CONFIG_VGPR_FREE_SAMPLE_TRACK_ID
		SampleTrackId = GetSampleTrackIdFromLaneIndex();
#endif

		// Matches first line of kStackowiakSampleSet0
		// TODO(Denoiser): could be optimised further by just setting sign bit on 0.5.
		float2 SampleOffset = float2(
			SampleTrackId & 0x1 ? 0.5 : -0.5,
			SampleTrackId & 0x2 ? 0.5 : -0.5);

		return KernelConfig.BufferUV + SampleOffset * KernelConfig.BufferSizeAndInvSize.zw;
	}

	return KernelConfig.BufferUV;
}

FSSDSampleSceneInfos UncompressRefSceneMetadata(FSSDKernelConfig KernelConfig)
{
	// Find out the buffer UV of the reference pixel.
	float2 RefBufferUV = ComputeRefBufferUV(KernelConfig);

	// Uncompress the reference scene metadata to keep a low VGPR pressure.
	return UncompressSampleSceneInfo(
		KernelConfig.RefSceneMetadataLayout, /* bIsPrevFrame = */ false,
		DenoiserBufferUVToScreenPosition(RefBufferUV),
		KernelConfig.CompressedRefSceneMetadata);
}

void StartAccumulatingCluster(
	FSSDKernelConfig KernelConfig,
	inout FSSDSignalAccumulatorArray UncompressedAccumulators,
	inout FSSDCompressedSignalAccumulatorArray CompressedAccumulators,
	FSSDSampleClusterInfo ClusterInfo)
{
	FSSDSampleSceneInfos RefSceneMetadata = UncompressRefSceneMetadata(KernelConfig);

#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION != ACCUMULATOR_COMPRESSION_DISABLED
	FSSDSignalAccumulatorArray Accumulators = UncompressAccumulatorArray(CompressedAccumulators, CONFIG_ACCUMULATOR_VGPR_COMPRESSION);
#endif

	[unrool(SIGNAL_ARRAY_SIZE)]
		for (uint SignalMultiplexId = 0; SignalMultiplexId < SIGNAL_ARRAY_SIZE; SignalMultiplexId++)
		{
#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION == ACCUMULATOR_COMPRESSION_DISABLED
			{
				StartAccumulatingCluster(
					RefSceneMetadata,
					/* inout */ UncompressedAccumulators.Array[SignalMultiplexId],
					ClusterInfo);
			}
#else
			{
				StartAccumulatingCluster(
					RefSceneMetadata,
					/* inout */ Accumulators.Array[SignalMultiplexId],
					ClusterInfo);
			}
#endif
		}

#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION != ACCUMULATOR_COMPRESSION_DISABLED
	CompressedAccumulators = CompressAccumulatorArray(Accumulators, CONFIG_ACCUMULATOR_VGPR_COMPRESSION);
#endif
}

//------------------------------------------------------- STACKOWIAK 2018

#if COMPILE_STACKOWIAK_KERNEL

static const float2 kStackowiakSampleSet0[56 * 4] =
{
	float2(-0.5, -0.5), float2(+0.5, -0.5), float2(-0.5, +0.5), float2(+0.5, +0.5),
	float2(-1.5, +0.5), float2(-1.5, -0.5), float2(-0.5, +1.5), float2(+1.5, -0.5),
	float2(+0.5, -1.5), float2(+2.5, -0.5), float2(+1.5, +0.5), float2(-0.5, -1.5),
	float2(-1.5, -2.5), float2(-0.5, -2.5), float2(-1.5, -1.5), float2(-0.5, +2.5),
	float2(-1.5, +1.5), float2(+1.5, -2.5), float2(-1.5, +2.5), float2(+1.5, +2.5),
	float2(+0.5, -2.5), float2(-2.5, -0.5), float2(-2.5, -1.5), float2(-2.5, +0.5),
	float2(+0.5, +1.5), float2(+0.5, +2.5), float2(-3.5, +0.5), float2(+0.5, +3.5),
	float2(+1.5, -1.5), float2(+3.5, -0.5), float2(+2.5, +1.5), float2(+3.5, +0.5),
	float2(+1.5, +1.5), float2(-2.5, +1.5), float2(-3.5, +2.5), float2(+3.5, +1.5),
	float2(-3.5, -0.5), float2(-1.5, -3.5), float2(-2.5, -2.5), float2(-2.5, +2.5),
	float2(+2.5, +0.5), float2(+2.5, +2.5), float2(+1.5, +3.5), float2(+3.5, -1.5),
	float2(-3.5, -2.5), float2(+3.5, -2.5), float2(+2.5, -1.5), float2(+0.5, -3.5),
	float2(-0.5, +3.5), float2(-0.5, -4.5), float2(-4.5, +0.5), float2(+4.5, +0.5),
	float2(-4.5, -1.5), float2(-3.5, +1.5), float2(-0.5, -3.5), float2(+1.5, -3.5),
	float2(+0.5, -4.5), float2(-1.5, +3.5), float2(+0.5, +4.5), float2(-3.5, -1.5),
	float2(-4.5, +1.5), float2(+2.5, -4.5), float2(+2.5, -2.5), float2(-1.5, +4.5),
	float2(-2.5, -4.5), float2(+4.5, -2.5), float2(+2.5, +3.5), float2(-3.5, +3.5),
	float2(-2.5, +3.5), float2(+0.5, -5.5), float2(-4.5, +3.5), float2(-2.5, -3.5),
	float2(-4.5, +2.5), float2(+3.5, +3.5), float2(+2.5, -3.5), float2(+4.5, +3.5),
	float2(+3.5, -3.5), float2(+4.5, +2.5), float2(-5.5, +1.5), float2(-4.5, -0.5),
	float2(+3.5, +2.5), float2(-0.5, +4.5), float2(-1.5, +5.5), float2(+1.5, +5.5),
	float2(+4.5, -0.5), float2(+5.5, +0.5), float2(+4.5, +1.5), float2(-1.5, -4.5),
	float2(-1.5, -5.5), float2(-4.5, -2.5), float2(-2.5, +5.5), float2(+2.5, +5.5),
	float2(+1.5, +4.5), float2(+5.5, +1.5), float2(+1.5, -4.5), float2(-3.5, -3.5),
	float2(+3.5, -4.5), float2(-3.5, -4.5), float2(+4.5, -1.5), float2(+4.5, -3.5),
	float2(-3.5, -5.5), float2(-2.5, -5.5), float2(-4.5, -3.5), float2(+4.5, +4.5),
	float2(-3.5, +4.5), float2(-2.5, +4.5), float2(-5.5, -2.5), float2(-5.5, +0.5),
	float2(+2.5, -5.5), float2(+3.5, +4.5), float2(-0.5, -5.5), float2(-0.5, +6.5),
	float2(+2.5, +4.5), float2(-5.5, -0.5), float2(-6.5, -1.5), float2(+1.5, -5.5),
	float2(-6.5, -0.5), float2(+0.5, +5.5), float2(+1.5, +6.5), float2(+6.5, +1.5),
	float2(-0.5, +5.5), float2(+6.5, -0.5), float2(-4.5, -4.5), float2(-5.5, +2.5),
	float2(+5.5, -0.5), float2(-5.5, -1.5), float2(-6.5, +3.5), float2(-1.5, +6.5),
	float2(-6.5, +0.5), float2(+4.5, -5.5), float2(-3.5, +6.5), float2(+6.5, -1.5),
	float2(+0.5, -6.5), float2(-5.5, -3.5), float2(+5.5, -2.5), float2(+4.5, -4.5),
	float2(+5.5, -1.5), float2(+3.5, -6.5), float2(+5.5, +3.5), float2(+3.5, -5.5),
	float2(-5.5, -4.5), float2(+6.5, -3.5), float2(-0.5, -6.5), float2(+3.5, +6.5),
	float2(-5.5, +3.5), float2(+0.5, +6.5), float2(+6.5, +0.5), float2(+6.5, -2.5),
	float2(-6.5, -3.5), float2(-4.5, +4.5), float2(-7.5, -0.5), float2(+7.5, +0.5),
	float2(+5.5, +2.5), float2(-0.5, -7.5), float2(+0.5, +7.5), float2(-4.5, +5.5),
	float2(+3.5, +5.5), float2(-3.5, +5.5), float2(-4.5, -5.5), float2(+4.5, +6.5),
	float2(+5.5, -4.5), float2(+4.5, +5.5), float2(-4.5, +6.5), float2(+6.5, +4.5),
	float2(-7.5, +1.5), float2(-6.5, +1.5), float2(+5.5, -3.5), float2(-6.5, +2.5),
	float2(-2.5, +6.5), float2(-1.5, -7.5), float2(+5.5, +4.5), float2(-1.5, -6.5),
	float2(-3.5, -7.5), float2(+2.5, -7.5), float2(-7.5, +2.5), float2(-6.5, -2.5),
	float2(-5.5, +5.5), float2(+2.5, +6.5), float2(-2.5, -6.5), float2(-7.5, +0.5),
	float2(-0.5, +7.5), float2(+7.5, -2.5), float2(-2.5, +7.5), float2(+0.5, -7.5),
	float2(-4.5, -7.5), float2(+7.5, +1.5), float2(+1.5, -6.5), float2(-6.5, +4.5),
	float2(-1.5, +7.5), float2(-5.5, -5.5), float2(+6.5, +2.5), float2(-3.5, -6.5),
	float2(+3.5, -7.5), float2(-5.5, +4.5), float2(+2.5, -6.5), float2(+1.5, -7.5),
	float2(+6.5, +3.5), float2(+5.5, -6.5), float2(-6.5, +5.5), float2(+7.5, +4.5),
	float2(+7.5, -1.5), float2(-7.5, -1.5), float2(+3.5, +7.5), float2(-5.5, +6.5),
	float2(+1.5, +7.5), float2(+7.5, +3.5), float2(+7.5, -0.5), float2(-7.5, -2.5),
	float2(+5.5, +5.5), float2(+6.5, +5.5), float2(+5.5, -5.5), float2(-2.5, -7.5),
	float2(+2.5, +7.5), float2(-7.5, -3.5), float2(-7.5, -4.5), float2(-6.5, -4.5),
	float2(+7.5, -3.5), float2(+5.5, +6.5), float2(-5.5, -6.5), float2(-4.5, -6.5),
	float2(+7.5, +2.5), float2(-7.5, +3.5), float2(+4.5, -6.5), float2(+7.5, -4.5),
};

static const float2 kStackowiakSampleSet1[56 * 4] =
{
	float2(-0.5, -0.5), float2(+0.5, -0.5), float2(-0.5, +0.5), float2(+0.5, +0.5),
	float2(+0.5, -1.5), float2(+1.5, -1.5), float2(-1.5, -0.5), float2(+1.5, +1.5),
	float2(-0.5, -2.5), float2(-1.5, -1.5), float2(+0.5, +1.5), float2(-1.5, +0.5),
	float2(+1.5, -0.5), float2(-0.5, +1.5), float2(-2.5, +0.5), float2(+0.5, +2.5),
	float2(-2.5, -1.5), float2(+2.5, +0.5), float2(+1.5, +0.5), float2(-0.5, -1.5),
	float2(-1.5, +1.5), float2(+2.5, -2.5), float2(-3.5, -0.5), float2(-1.5, +2.5),
	float2(-2.5, +1.5), float2(-2.5, -0.5), float2(-1.5, -2.5), float2(+2.5, -1.5),
	float2(-3.5, +0.5), float2(-0.5, -3.5), float2(-1.5, +3.5), float2(+0.5, -2.5),
	float2(+1.5, +2.5), float2(-0.5, +2.5), float2(+0.5, +3.5), float2(+3.5, +0.5),
	float2(+2.5, +1.5), float2(-2.5, -2.5), float2(+2.5, -0.5), float2(+3.5, -1.5),
	float2(-0.5, +3.5), float2(+3.5, +1.5), float2(-3.5, +2.5), float2(+3.5, +2.5),
	float2(+3.5, -0.5), float2(+0.5, -4.5), float2(-2.5, +3.5), float2(+0.5, -3.5),
	float2(-1.5, -4.5), float2(+1.5, +3.5), float2(+1.5, -2.5), float2(-3.5, +1.5),
	float2(+2.5, -3.5), float2(-2.5, -3.5), float2(+2.5, +2.5), float2(+1.5, +4.5),
	float2(-4.5, -2.5), float2(-2.5, +2.5), float2(-4.5, +1.5), float2(+4.5, +1.5),
	float2(-2.5, -4.5), float2(+3.5, -3.5), float2(-1.5, -3.5), float2(-3.5, -1.5),
	float2(+1.5, -4.5), float2(+4.5, -2.5), float2(+1.5, -3.5), float2(-1.5, +4.5),
	float2(-4.5, +2.5), float2(-4.5, -0.5), float2(+2.5, +4.5), float2(-4.5, +0.5),
	float2(-3.5, -4.5), float2(+0.5, +4.5), float2(+3.5, -2.5), float2(-3.5, -2.5),
	float2(-3.5, +3.5), float2(+3.5, +3.5), float2(+4.5, +0.5), float2(+0.5, +5.5),
	float2(-0.5, +4.5), float2(+4.5, -3.5), float2(-1.5, +5.5), float2(-0.5, -4.5),
	float2(+2.5, +3.5), float2(+4.5, +2.5), float2(-2.5, +5.5), float2(+2.5, -4.5),
	float2(+4.5, -0.5), float2(+5.5, -0.5), float2(-4.5, +4.5), float2(+5.5, -1.5),
	float2(-5.5, -1.5), float2(-4.5, -1.5), float2(+3.5, +4.5), float2(-3.5, -3.5),
	float2(-5.5, +0.5), float2(+1.5, -5.5), float2(-5.5, -2.5), float2(-3.5, +4.5),
	float2(+0.5, -5.5), float2(-2.5, -5.5), float2(+2.5, +5.5), float2(+4.5, +4.5),
	float2(+4.5, -1.5), float2(-2.5, +4.5), float2(+4.5, +3.5), float2(+0.5, +6.5),
	float2(-0.5, -6.5), float2(+5.5, +2.5), float2(-0.5, -5.5), float2(-5.5, -0.5),
	float2(-6.5, -1.5), float2(-0.5, +5.5), float2(-0.5, +6.5), float2(+6.5, -0.5),
	float2(+1.5, +5.5), float2(+1.5, -6.5), float2(+5.5, +0.5), float2(-5.5, +2.5),
	float2(+5.5, +1.5), float2(-5.5, +1.5), float2(-6.5, -0.5), float2(-1.5, -5.5),
	float2(-5.5, -4.5), float2(-4.5, +3.5), float2(-6.5, +1.5), float2(+2.5, -5.5),
	float2(+3.5, -5.5), float2(-5.5, -3.5), float2(+1.5, +6.5), float2(+6.5, +2.5),
	float2(+4.5, -4.5), float2(+3.5, -6.5), float2(-4.5, -4.5), float2(-4.5, -3.5),
	float2(-6.5, +2.5), float2(+3.5, +5.5), float2(+3.5, -4.5), float2(+5.5, -3.5),
	float2(-5.5, +4.5), float2(+6.5, -3.5), float2(-6.5, -2.5), float2(+5.5, +4.5),
	float2(-1.5, +6.5), float2(-0.5, -7.5), float2(-6.5, +3.5), float2(-5.5, +3.5),
	float2(-6.5, -4.5), float2(+7.5, -1.5), float2(-3.5, -5.5), float2(+3.5, +6.5),
	float2(+5.5, +3.5), float2(+7.5, +0.5), float2(+5.5, -2.5), float2(-6.5, +0.5),
	float2(-7.5, +1.5), float2(-3.5, -6.5), float2(+6.5, +0.5), float2(+7.5, +1.5),
	float2(-2.5, -7.5), float2(-3.5, +5.5), float2(-7.5, -0.5), float2(-3.5, +6.5),
	float2(-2.5, +6.5), float2(+4.5, -6.5), float2(-5.5, +5.5), float2(+4.5, -5.5),
	float2(+6.5, -2.5), float2(+6.5, +3.5), float2(-1.5, -6.5), float2(-1.5, +7.5),
	float2(+6.5, +1.5), float2(-5.5, -5.5), float2(+0.5, -6.5), float2(+7.5, +3.5),
	float2(+2.5, +6.5), float2(-4.5, +5.5), float2(-6.5, -3.5), float2(-4.5, -5.5),
	float2(-6.5, -5.5), float2(+5.5, -6.5), float2(-2.5, -6.5), float2(+5.5, -5.5),
	float2(+4.5, +5.5), float2(-7.5, +0.5), float2(+6.5, -1.5), float2(+0.5, -7.5),
	float2(+7.5, -0.5), float2(-3.5, -7.5), float2(+2.5, -6.5), float2(-3.5, +7.5),
	float2(-4.5, -7.5), float2(-0.5, +7.5), float2(-6.5, +5.5), float2(+7.5, -3.5),
	float2(-4.5, +6.5), float2(+1.5, +7.5), float2(+5.5, -4.5), float2(+7.5, +4.5),
	float2(+0.5, +7.5), float2(+4.5, +6.5), float2(-4.5, +7.5), float2(-7.5, -1.5),
	float2(+3.5, -7.5), float2(+7.5, -4.5), float2(+3.5, +7.5), float2(-1.5, -7.5),
	float2(+6.5, -4.5), float2(-7.5, -3.5), float2(+6.5, +4.5), float2(+2.5, -7.5),
	float2(+7.5, -2.5), float2(-7.5, +2.5), float2(+1.5, -7.5), float2(-5.5, +6.5),
	float2(+5.5, +5.5), float2(-2.5, +7.5), float2(+7.5, +2.5), float2(-7.5, -2.5),
	float2(+2.5, +7.5), float2(-6.5, +4.5), float2(+5.5, +6.5), float2(-4.5, -6.5),
};

static const uint kStackowiakSampleSetCount = 4;
static const uint kStackowiakSampleCountPerSet = 56;

void ConvolveStackowiakKernel(
	FSSDKernelConfig KernelConfig,
	FSSDTexture2D SignalBuffer0,
	FSSDTexture2D SignalBuffer1,
	FSSDTexture2D SignalBuffer2,
	FSSDTexture2D SignalBuffer3,
	inout FSSDSignalAccumulatorArray UncompressedAccumulators,
	inout FSSDCompressedSignalAccumulatorArray CompressedAccumulators)
{
	// Number of batch size done at same time, to improve lattency hidding.
	const uint kSamplingBatchSize = 2;

	if (KernelConfig.bDescOrder)
	{
		// (SALU) Number of batch of samples to perform.
		const uint BatchCountCount = (KernelConfig.SampleCount + (kSamplingBatchSize - 1)) / kSamplingBatchSize;

		// (SALU) Compute a final number of sample quantize the sampling batch size.
		const uint SampleCount = BatchCountCount * kSamplingBatchSize;

		// Compile time number of samples between rings.
		const uint StocasticSamplesPerCluster = 8 / kStackowiakSampleSetCount;

		// Compute the first index at witch digestion must happen.
		uint CurrentRingId = 0;
		uint NextClusterBoundary = 0;

		if (StocasticSamplesPerCluster == 2)
		{
			uint un = SampleCount - 1;

			CurrentRingId = (uint(floor(sqrt(4 * un - 3))) + 1) / 2;

			NextClusterBoundary = 1 + CurrentRingId * (CurrentRingId - 1);
		}
		else
		{
			// TODO(Denoiser)
		}

		FSSDSampleClusterInfo ClusterInfo;
		ClusterInfo.OutterBoundaryRadius = (CurrentRingId + 1) * KernelConfig.KernelSpreadFactor;

		StartAccumulatingCluster(
			KernelConfig,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators,
			ClusterInfo);

		// Processes the samples in batches so that the compiler can do lattency hidding.
		[loop]
			for (uint BatchId = 0; BatchId < BatchCountCount; BatchId++)
			{
				[unrool(2)]
					for (uint SampleBatchId = 0; SampleBatchId < 2; SampleBatchId++)
					{
						uint SampleId = (BatchCountCount - BatchId) * kSamplingBatchSize - 1 - SampleBatchId;

						bool bIsKernelCenterSample = SampleId == 0 && (SampleBatchId == (kSamplingBatchSize - 1));

						uint SampleTrackId = KernelConfig.SampleTrackId;
#if CONFIG_VGPR_FREE_SAMPLE_TRACK_ID
						SampleTrackId = GetSampleTrackIdFromLaneIndex();
#endif

						float2 SampleOffset = kStackowiakSampleSet0[kStackowiakSampleSetCount * SampleId + SampleTrackId];
						if (KernelConfig.SampleSubSetId == 1)
						{
							SampleOffset = kStackowiakSampleSet1[kStackowiakSampleSetCount * SampleId + SampleTrackId];
						}

						float2 SampleBufferUV = KernelConfig.BufferUV + (SampleOffset * KernelConfig.KernelSpreadFactor) * KernelConfig.BufferSizeAndInvSize.zw;

						float KernelWeight = 1;
						SampleAndAccumulateMultiplexedSignals(
							KernelConfig,
							SignalBuffer0,
							SignalBuffer1,
							SignalBuffer2,
							SignalBuffer3,
							/* inout */ UncompressedAccumulators,
							/* inout */ CompressedAccumulators,
							SampleBufferUV,
							/* MipLevel = */ 0.0,
							/* KernelWeight = */ 1.0,
							/* bForceSample = */ bIsKernelCenterSample && KernelConfig.bForceKernelCenterAccumulation);

						// Change of cluster. Can only happens on odd SampleId, meaning even SampleBatchId.
						[branch]
							if (SampleId == NextClusterBoundary && (SampleBatchId % 2) == 0)
							{
								// Compute the number samples that have been accumulated for this sample.
								uint SampleCountForCluster = min(CurrentRingId * StocasticSamplesPerCluster, SampleCount - SampleId);

								// Digest all acumulators.
								DijestAccumulatedClusterSamples(
									/* inout */ UncompressedAccumulators,
									/* inout */ CompressedAccumulators,
									CurrentRingId, SampleCountForCluster);

								BRANCH
									if (!KernelConfig.bSampleKernelCenter && SampleId == 1)
									{
										break;
									}

								// Change cluster index and boundary.
								CurrentRingId -= 1;
								NextClusterBoundary -= CurrentRingId * StocasticSamplesPerCluster;

								FSSDSampleClusterInfo ClusterInfo;
								ClusterInfo.OutterBoundaryRadius = (CurrentRingId + 1) * KernelConfig.KernelSpreadFactor;

								// Prepare the accumulators for new cluster.
								StartAccumulatingCluster(
									KernelConfig,
									/* inout */ UncompressedAccumulators,
									/* inout */ CompressedAccumulators,
									ClusterInfo);
							}
					} // for (uint SampleBatchId = 0; SampleBatchId < kSamplingBatchSize; SampleBatchId++)
			} // for (uint BatchId = 0; BatchId < BatchCountCount; BatchId++)

			// NextClusterBoundary is not capable to reach 0, therefore need to manually digest the center sample.
		if (KernelConfig.bSampleKernelCenter)
		{
			DijestAccumulatedClusterSamples(
				/* inout */ UncompressedAccumulators,
				/* inout */ CompressedAccumulators,
				/* RingId = */ 0, /* SampleCount = */ 1);
		}
	}
	else // if (!KernelConfig.bDescOrder)
	{
		if (KernelConfig.bSampleKernelCenter)
		{
			SampleAndAccumulateCenterSampleAsItsOwnCluster(
				KernelConfig,
				SignalBuffer0,
				SignalBuffer1,
				SignalBuffer2,
				SignalBuffer3,
				/* inout */ UncompressedAccumulators,
				/* inout */ CompressedAccumulators);
		}

		// Accumulate second sample to lattency hide with the center sample.
		{
			uint SampleTrackId = KernelConfig.SampleTrackId;
#if CONFIG_VGPR_FREE_SAMPLE_TRACK_ID
			SampleTrackId = GetSampleTrackIdFromLaneIndex();
#endif

			uint SampleId = 1;

			float2 SampleOffset = kStackowiakSampleSet0[kStackowiakSampleSetCount * SampleId + SampleTrackId];
			if (KernelConfig.SampleSubSetId == 1)
			{
				SampleOffset = kStackowiakSampleSet1[kStackowiakSampleSetCount * SampleId + SampleTrackId];
			}

			float2 SampleBufferUV = KernelConfig.BufferUV + (SampleOffset * KernelConfig.KernelSpreadFactor) * KernelConfig.BufferSizeAndInvSize.zw;

			SampleAndAccumulateMultiplexedSignals(
				KernelConfig,
				SignalBuffer0,
				SignalBuffer1,
				SignalBuffer2,
				SignalBuffer3,
				/* inout */ UncompressedAccumulators,
				/* inout */ CompressedAccumulators,
				SampleBufferUV,
				/* MipLevel = */ 0.0,
				/* KernelWeight = */ 1.0,
				/* bForceSample = */ false);
		}

		// (SALU) Number of batch of samples to perform.
		const uint BatchCountCount = (KernelConfig.SampleCount - 1) / kSamplingBatchSize;

		// Processes the samples in batches so that the compiler can do lattency hidding.
		// TODO(Denoiser): kSamplingBatchSize for lattency hidding
		LOOP
			for (uint BatchId = 0; BatchId < BatchCountCount; BatchId++)
			{
				float2 SampleBufferUV[2];

				uint SampleTrackId = KernelConfig.SampleTrackId;
#if CONFIG_VGPR_FREE_SAMPLE_TRACK_ID
				SampleTrackId = GetSampleTrackIdFromLaneIndex();
#endif

				[unroll(2)]
					for (uint SampleBatchId = 0; SampleBatchId < 2; SampleBatchId++)
					{
						uint SampleId = BatchId * kSamplingBatchSize + (SampleBatchId + kSamplingBatchSize);

						float2 SampleOffset = kStackowiakSampleSet0[kStackowiakSampleSetCount * SampleId + SampleTrackId];
						if (KernelConfig.SampleSubSetId == 1)
						{
							SampleOffset = kStackowiakSampleSet1[kStackowiakSampleSetCount * SampleId + SampleTrackId];
						}

						SampleBufferUV[SampleBatchId] = KernelConfig.BufferUV + (SampleOffset * KernelConfig.KernelSpreadFactor) * KernelConfig.BufferSizeAndInvSize.zw;
					}

				SampleAndAccumulateMultiplexedSignalsPair(
					KernelConfig,
					SignalBuffer0,
					SignalBuffer1,
					SignalBuffer2,
					SignalBuffer3,
					/* inout */ UncompressedAccumulators,
					/* inout */ CompressedAccumulators,
					SampleBufferUV,
					/* KernelWeight = */ 1.0);
			} // for (uint BatchId = 0; BatchId < BatchCountCount; BatchId++)
	} // if (!KernelConfig.bDescOrder)
} // ConvolveStackowiakKernel()

#endif // COMPILE_STACKOWIAK_KERNEL

void AccumulateKernel(
	FSSDKernelConfig KernelConfig,
	FSSDTexture2D SignalBuffer0,
	FSSDTexture2D SignalBuffer1,
	FSSDTexture2D SignalBuffer2,
	FSSDTexture2D SignalBuffer3,
	inout FSSDSignalAccumulatorArray UncompressedAccumulators,
	inout FSSDCompressedSignalAccumulatorArray CompressedAccumulators)
{
	if (KernelConfig.SampleSet == 0xDEADDEAD)
	{
	}
#if COMPILE_BOX_KERNEL
	else if (KernelConfig.SampleSet == SAMPLE_SET_1X1)
	{
		if (KernelConfig.bSampleKernelCenter)
		{
			SampleAndAccumulateCenterSampleAsItsOwnCluster(
				KernelConfig,
				SignalBuffer0,
				SignalBuffer1,
				SignalBuffer2,
				SignalBuffer3,
				/* inout */ UncompressedAccumulators,
				/* inout */ CompressedAccumulators);
		}
	}
	else if (KernelConfig.SampleSet == SAMPLE_SET_2X2_BILINEAR)
	{
		AccumulateBilinear(
			KernelConfig,
			SignalBuffer0,
			SignalBuffer1,
			SignalBuffer2,
			SignalBuffer3,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators);
	}
	else if (KernelConfig.SampleSet == SAMPLE_SET_2X2_STOCASTIC)
	{
		AccumulateStocasticBilinear(
			KernelConfig,
			SignalBuffer0,
			SignalBuffer1,
			SignalBuffer2,
			SignalBuffer3,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators);
	}
	else if (
		KernelConfig.SampleSet == SAMPLE_SET_3X3 ||
		KernelConfig.SampleSet == SAMPLE_SET_3X3_PLUS ||
		KernelConfig.SampleSet == SAMPLE_SET_3X3_CROSS)
	{
		AccumulateSquare3x3Kernel(
			KernelConfig,
			SignalBuffer0,
			SignalBuffer1,
			SignalBuffer2,
			SignalBuffer3,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators);
	}
	else if (
		KernelConfig.SampleSet == SAMPLE_SET_3X3_SOBEK2018 ||
		KernelConfig.SampleSet == SAMPLE_SET_5X5_WAVELET ||
		KernelConfig.SampleSet == SAMPLE_SET_NXN)
	{
		AccumulateSquareKernel(
			KernelConfig,
			SignalBuffer0,
			SignalBuffer1,
			SignalBuffer2,
			SignalBuffer3,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators);
	}
#endif//  COMPILE_BOX_KERNEL
#if COMPILE_STACKOWIAK_KERNEL
	else if (KernelConfig.SampleSet == SAMPLE_SET_STACKOWIAK_4_SETS)
	{
		ConvolveStackowiakKernel(
			KernelConfig,
			SignalBuffer0,
			SignalBuffer1,
			SignalBuffer2,
			SignalBuffer3,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators);
	}
#endif // COMPILE_STACKOWIAK_KERNEL
#if COMPILE_DISK_KERNEL
	else if (KernelConfig.SampleSet == SAMPLE_SET_HEXAWEB)
	{
		ConvolveDiskKernel(
			KernelConfig,
			SignalBuffer0,
			SignalBuffer1,
			SignalBuffer2,
			SignalBuffer3,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators);
	}
#endif // COMPILE_DISK_KERNEL
#if COMPILE_HEIRARCHY_KERNEL
	else if (KernelConfig.SampleSet == SAMPLE_SET_STOCASTIC_HIERARCHY)
	{
		ConvolveStocasticHierarchy(
			KernelConfig,
			SignalBuffer0,
			SignalBuffer1,
			SignalBuffer2,
			SignalBuffer3,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators);
	}
#endif // COMPILE_HEIRARCHY_KERNEL
#if COMPILE_DIRECTIONAL_KERNEL
	else if (KernelConfig.SampleSet == SAMPLE_SET_DIRECTIONAL_RECT || KernelConfig.SampleSet == SAMPLE_SET_DIRECTIONAL_ELLIPSE)
	{
		ConvolveDirectionalRect(
			KernelConfig,
			SignalBuffer0,
			SignalBuffer1,
			SignalBuffer2,
			SignalBuffer3,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators);
	}
#endif // COMPILE_DIRECTIONAL_KERNEL
} // AccumulateKernel()

#endif