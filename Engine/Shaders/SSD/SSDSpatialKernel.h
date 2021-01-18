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
	else if (BilateralPresetId == BILATERAL_PRESET_POLYCHROMATIC_PENUMBRA)
	{
		// Diffuse.
		KernelConfig.BilateralSettings[0] = BILATERAL_POSITION_BASED(5) | BILATERAL_NORMAL;

		// Specular.
#if SIGNAL_ARRAY_SIZE > 1
		KernelConfig.BilateralSettings[1] = BILATERAL_POSITION_BASED(5) | BILATERAL_TOKOYASHI;
#endif
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

/** Uncompress the scene metadata of a sample. */
FSSDSampleSceneInfos UncompressSampleSceneMetadata(
	FSSDKernelConfig KernelConfig,
	float2 SampleBufferUV,
	FSSDCompressedSceneInfos CompressedSampleSceneMetadata)
{
	return UncompressSampleSceneInfo(
		CONFIG_METADATA_BUFFER_LAYOUT, KernelConfig.bPreviousFrameMetadata,
		DenoiserBufferUVToScreenPosition(SampleBufferUV),
		CompressedSampleSceneMetadata);
}

float3 ComputeVectorFromNeighborToRef(
	FSSDKernelConfig KernelConfig,
	FSSDSampleSceneInfos RefSceneMetadata,
	FSSDSampleSceneInfos NeighborSceneMetadata)
{
	float RefWorldDepth = GetWorldDepth(RefSceneMetadata);
	float NeighborWorldDepth = GetWorldDepth(NeighborSceneMetadata);

	if (KernelConfig.NeighborToRefComputation == NEIGHBOR_TO_REF_LOWEST_VGPR_PRESSURE)
	{
		// Recompute the the screen position of the reference, from the most minimal VGPR footprint.
		float2 RefScreenPos = RefSceneMetadata.ScreenPosition;
		float3 RefClipPosition = float3(RefScreenPos * (ViewToClip[3][3] < 1.0f ? RefWorldDepth : 1.0f), RefWorldDepth);

		float2 NeighborScreenPos = NeighborSceneMetadata.ScreenPosition;
		float3 NeighborClipPosition = float3(NeighborScreenPos * (ViewToClip[3][3] < 1.0f ? NeighborWorldDepth : 1.0f), NeighborWorldDepth);

#if CONFIG_USE_VIEW_SPACE
		float3 NeighborToRefVector = mul(float4(RefClipPosition - NeighborClipPosition, 0), GetScreenToViewDistanceMatrix()).xyz;
#else
		float3 NeighborToRefVector = mul(float4(RefClipPosition - NeighborClipPosition, 0),ScreenToTranslatedWorld).xyz;
#endif

		return NeighborToRefVector;
	}
	else // if (KernelConfig.NeighborToRefComputation == NEIGHBOR_TO_REF_CACHE_WORLD_POSITION)
	{
		//error
		return 0;
	}
}

/** Returns whether this sample is outside the viewport. */
bool IsOutsideViewport(FSSDKernelConfig KernelConfig, float2 SampleBufferUV)
{
	return any(SampleBufferUV < KernelConfig.BufferBilinearUVMinMax.xy || SampleBufferUV > KernelConfig.BufferBilinearUVMinMax.zw);
}

/** Sample multiplexed samples and their metadata for kernel use. */
void SampleMultiplexedSignals(
	FSSDKernelConfig KernelConfig,
	FSSDTexture2D SignalBuffer0,
	FSSDTexture2D SignalBuffer1,
	FSSDTexture2D SignalBuffer2,
	FSSDTexture2D SignalBuffer3,
	float2 SampleBufferUV,
	float MipLevel,
	out FSSDCompressedSceneInfos OutCompressedSampleSceneMetadata,
	out FSSDCompressedMultiplexedSample OutCompressedMultiplexedSamples)
{
	uint2 PixelCoord = BufferUVToBufferPixelCoord(SampleBufferUV);

	OutCompressedSampleSceneMetadata = SampleCompressedSceneMetadata(
		KernelConfig.bPreviousFrameMetadata, SampleBufferUV, PixelCoord);

	// Fetches the signals sample
	OutCompressedMultiplexedSamples = SampleCompressedMultiplexedSignals(
		SignalBuffer0,
		SignalBuffer1,
		SignalBuffer2,
		SignalBuffer3,
		GlobalPointClampedSampler,
		SampleBufferUV,
		KernelConfig.BufferMipLevel + MipLevel,
		PixelCoord);
} // SampleMultiplexedSignals()

/** Uncompressed multiplexed signal for accumulation. */
FSSDSignalArray UncompressMultiplexedSignals(
	FSSDKernelConfig KernelConfig,
	float2 SampleBufferUV,
	FSSDCompressedMultiplexedSample CompressedMultiplexedSamples)
{
	// TODO(Denoiser): offer multiplier to apply to each signal during Decode, to save mul VALU.
	FSSDSignalArray MultiplexedSamples = DecodeMultiplexedSignals(
		KernelConfig.BufferLayout,
		/* MultiplexedSampleId = */ 0,
		KernelConfig.bNormalizeSample,
		CompressedMultiplexedSamples);

	if (KernelConfig.bClampUVPerMultiplexedSignal)
	{
		[unroll(SIGNAL_ARRAY_SIZE)]
			for (uint SignalMultiplexId = 0; SignalMultiplexId < SIGNAL_ARRAY_SIZE; SignalMultiplexId++)
			{
				bool bInvalidSample = any(SampleBufferUV != clamp(
					SampleBufferUV, KernelConfig.PerSignalUVMinMax[SignalMultiplexId].xy, KernelConfig.PerSignalUVMinMax[SignalMultiplexId].zw));

				if (bInvalidSample)
				{
					MultiplexedSamples.Array[SignalMultiplexId] = CreateSignalSampleFromScalarValue(0.0);
				}
			} // for (uint SignalMultiplexId = 0; SignalMultiplexId < SIGNAL_ARRAY_SIZE; SignalMultiplexId++)
	}

	return MultiplexedSamples;
}

/** Accumulate multiplexed samples and their metadata to an accumulator. */
void AccumulateSampledMultiplexedSignals(
	FSSDKernelConfig KernelConfig,
	inout FSSDSignalAccumulatorArray UncompressedAccumulators,
	inout FSSDCompressedSignalAccumulatorArray CompressedAccumulators,
	FSSDSampleSceneInfos RefSceneMetadata,
	float2 SampleBufferUV,
	FSSDSampleSceneInfos SampleSceneMetadata,
	FSSDSignalArray MultiplexedSamples,
	float KernelSampleWeight,
	const bool bForceSample,
	bool bIsOutsideFrustum)
{
	// Compute the bluring radius of the output pixel itself.
	float RefPixelWorldBluringRadius = ComputeWorldBluringRadiusCausedByPixelSize(RefSceneMetadata);

#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION != ACCUMULATOR_COMPRESSION_DISABLED
	FSSDSignalAccumulatorArray Accumulators = UncompressAccumulatorArray(CompressedAccumulators, CONFIG_ACCUMULATOR_VGPR_COMPRESSION);
#endif

	// Compute the vector from neighbor to reference in the most optimal way.
	float3 NeighborToRefVector = ComputeVectorFromNeighborToRef(
		KernelConfig,
		RefSceneMetadata,
		SampleSceneMetadata);

	[unroll(SIGNAL_ARRAY_SIZE)]
		for (uint SignalMultiplexId = 0; SignalMultiplexId < SIGNAL_ARRAY_SIZE; SignalMultiplexId++)
		{
			// Compute at compile time the id of the signal being processed.
			const uint BatchedSignalId = ComputeSignalBatchIdFromSignalMultiplexId(KernelConfig, SignalMultiplexId);

			// TODO(Denoiser): direction of the ray should be cached by injest or output by RGS, otherwise ends up with VGPR pressure because of SampleBufferUV.
			uint2 NeighborPixelCoord = floor(SampleBufferUV * KernelConfig.BufferSizeAndInvSize.xy);

			// Fetch and pre process the sample for accumulation.
			FSSDSignalSample Sample = MultiplexedSamples.Array[SignalMultiplexId];
			Sample = TransformSignalSampleForAccumulation(KernelConfig, SignalMultiplexId, SampleSceneMetadata, Sample, NeighborPixelCoord);

			// Compute the bluring radius of pixel itself.
			float SamplePixelWorldBluringRadius = ComputeWorldBluringRadiusCausedByPixelSize(SampleSceneMetadata);

			// Compute the bluring radius of the signal from ray hit distance and signal domain knowledge.
			float SignalConvolutionBluringRadius = GetSignalWorldBluringRadius(Sample, SampleSceneMetadata, BatchedSignalId);

			// But the signal's bluring radius might already be pre computed.
			if (KernelConfig.BilateralDistanceComputation == SIGNAL_WORLD_FREQUENCY_PRECOMPUTED_BLURING_RADIUS)
			{
				// TODO(Denoiser): this is ineficient, could fetch the normalised WorldBluringRadius instead of SafeRcp().
				SignalConvolutionBluringRadius = Sample.WorldBluringRadius * SafeRcp(Sample.SampleCount);
			}

			// Compute the final world distance to use for bilateral rejection.
			float FinalWorldBluringDistance = -1;
			if (KernelConfig.BilateralDistanceComputation == SIGNAL_WORLD_FREQUENCY_REF_METADATA_ONLY)
			{
				FinalWorldBluringDistance = AmendWorldBluringRadiusCausedByPixelSize(
					RefPixelWorldBluringRadius);
			}
			else if (KernelConfig.BilateralDistanceComputation == SIGNAL_WORLD_FREQUENCY_SAMPLE_METADATA_ONLY)
			{
				FinalWorldBluringDistance = AmendWorldBluringRadiusCausedByPixelSize(
					SamplePixelWorldBluringRadius);
			}
			else if (KernelConfig.BilateralDistanceComputation == SIGNAL_WORLD_FREQUENCY_MIN_METADATA)
			{
				FinalWorldBluringDistance = AmendWorldBluringRadiusCausedByPixelSize(
					min(SamplePixelWorldBluringRadius, RefPixelWorldBluringRadius));
			}
			else if (
				KernelConfig.BilateralDistanceComputation == SIGNAL_WORLD_FREQUENCY_HIT_DISTANCE ||
				KernelConfig.BilateralDistanceComputation == SIGNAL_WORLD_FREQUENCY_PRECOMPUTED_BLURING_RADIUS)
			{
				FinalWorldBluringDistance = SignalConvolutionBluringRadius;
			}
			else if (KernelConfig.BilateralDistanceComputation == SIGNAL_WORLD_FREQUENCY_HARMONIC)
			{
				FinalWorldBluringDistance = AmendWorldBluringRadiusCausedByPixelSize(
					RefPixelWorldBluringRadius) * KernelConfig.HarmonicPeriode;
			}

			FinalWorldBluringDistance *= KernelConfig.WorldBluringDistanceMultiplier;

			if (KernelConfig.bMaxWithRefBilateralDistance)
			{
				FinalWorldBluringDistance = min(FinalWorldBluringDistance, KernelConfig.RefBilateralDistance[SignalMultiplexId]);
			}

			// Compute the weight to be applied to do bilateral rejection.
			float BilateralWeight = ComputeBilateralWeight(
				KernelConfig.BilateralSettings[SignalMultiplexId],
				FinalWorldBluringDistance,
				RefSceneMetadata,
				SampleSceneMetadata,
				NeighborToRefVector);

			float RatioEstimatorWeight = GetRatioEstimatorWeight(
				RefSceneMetadata, SampleSceneMetadata, Sample, NeighborPixelCoord);

			FSSDSampleAccumulationInfos SampleInfos;
			SampleInfos.Sample = Sample;
			SampleInfos.FinalWeight = KernelSampleWeight * BilateralWeight * RatioEstimatorWeight;
			SampleInfos.InvFrequency = SignalConvolutionBluringRadius;

			if (bForceSample || KernelConfig.bForceAllAccumulation)
			{
				SampleInfos.FinalWeight = 1;
			}

			// TODO(Denoiser): bIsOutsideFrustum could afect number of samples for DRB.
			[flatten]
				if (SampleInfos.Sample.SampleCount != 0 && !bIsOutsideFrustum)
				{
#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION == ACCUMULATOR_COMPRESSION_DISABLED
					{
						AccumulateSample(
							/* inout */ UncompressedAccumulators.Array[SignalMultiplexId],
							SampleInfos);
					}
#else
					{
						AccumulateSample(
							/* inout */ Accumulators.Array[SignalMultiplexId],
							SampleInfos);
					}
#endif
				}
		} // for (uint SignalMultiplexId = 0; SignalMultiplexId < SIGNAL_ARRAY_SIZE; SignalMultiplexId++)

#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION != ACCUMULATOR_COMPRESSION_DISABLED
	CompressedAccumulators = CompressAccumulatorArray(Accumulators, CONFIG_ACCUMULATOR_VGPR_COMPRESSION);
#endif
} // AccumulateSampledMultiplexedSignals().

/** Sample and accumulate to accumulatore array.
 *
 * Caution: you probably want to explicitly do this manually to help the shader compiler to do lattency hiding.
 */
void SampleAndAccumulateMultiplexedSignals(
	FSSDKernelConfig KernelConfig,
	FSSDTexture2D SignalBuffer0,
	FSSDTexture2D SignalBuffer1,
	FSSDTexture2D SignalBuffer2,
	FSSDTexture2D SignalBuffer3,
	inout FSSDSignalAccumulatorArray UncompressedAccumulators,
	inout FSSDCompressedSignalAccumulatorArray CompressedAccumulators,
	float2 SampleBufferUV,
	float MipLevel,
	float KernelSampleWeight,
	const bool bForceSample)
{
	// Stores in SGPR whether this sample is outside the viewport, to avoid VGPR pressure to keep SampleBufferUV after texture fetches.
	bool bIsOutsideFrustum = IsOutsideViewport(KernelConfig, SampleBufferUV);

	FSSDCompressedSceneInfos CompressedSampleSceneMetadata;
	FSSDCompressedMultiplexedSample CompressedMultiplexedSamples;

	// Force all the signal texture fetch and metadata to overlap to minimize serial texture fetches.
	{
		SampleMultiplexedSignals(
			KernelConfig,
			SignalBuffer0,
			SignalBuffer1,
			SignalBuffer2,
			SignalBuffer3,
			SampleBufferUV,
			MipLevel,
			/* out */ CompressedSampleSceneMetadata,
			/* out */ CompressedMultiplexedSamples);
	}

		// Accumulate the samples, giving full freedom for shader compiler scheduler to put instructions in most optimal way.
	{
		FSSDSignalArray MultiplexedSamples = UncompressMultiplexedSignals(
			KernelConfig, SampleBufferUV, CompressedMultiplexedSamples);

		FSSDSampleSceneInfos RefSceneMetadata = UncompressRefSceneMetadata(KernelConfig);

		FSSDSampleSceneInfos SampleSceneMetadata = UncompressSampleSceneMetadata(
			KernelConfig, SampleBufferUV, CompressedSampleSceneMetadata);

		AccumulateSampledMultiplexedSignals(
			KernelConfig,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators,
			RefSceneMetadata,
			SampleBufferUV,
			SampleSceneMetadata,
			MultiplexedSamples,
			KernelSampleWeight,
			bForceSample,
			bIsOutsideFrustum);
	}
} // SampleAndAccumulateMultiplexedSignals()

void SampleAndAccumulateMultiplexedSignalsPair(
	FSSDKernelConfig KernelConfig,
	FSSDTexture2D SignalBuffer0,
	FSSDTexture2D SignalBuffer1,
	FSSDTexture2D SignalBuffer2,
	FSSDTexture2D SignalBuffer3,
	inout FSSDSignalAccumulatorArray UncompressedAccumulators,
	inout FSSDCompressedSignalAccumulatorArray CompressedAccumulators,
	float2 SampleBufferUV[2],
	float KernelSampleWeight)
{
	FSSDCompressedSceneInfos CompressedSampleSceneMetadata[2];
	FSSDCompressedMultiplexedSample CompressedMultiplexedSamples[2];
	bool bIsOutsideFrustum[2];

	// Force all the signal texture fetch and metadata to overlap to minimize serial texture fetches.
	{
		[unroll(2)]
		for (uint PairFetchId = 0; PairFetchId < 2; PairFetchId++)
		{
			// Stores in SGPR whether this sample is outside the viewport, to avoid VGPR pressure to
			// avoid keeping SampleBufferUV after texture fetches.
			bIsOutsideFrustum[PairFetchId] = IsOutsideViewport(KernelConfig, SampleBufferUV[PairFetchId]);

			SampleMultiplexedSignals(
				KernelConfig,
				SignalBuffer0,
				SignalBuffer1,
				SignalBuffer2,
				SignalBuffer3,
				SampleBufferUV[PairFetchId],
				/* MipLevel = */ 0.0,
				/* out */ CompressedSampleSceneMetadata[PairFetchId],
				/* out */ CompressedMultiplexedSamples[PairFetchId]);
		}
	}

		// Accumulate the samples, giving full freedom for shader compiler scheduler to put instructions in most optimal way.
	{
		// Uncompress the multiplexed signal.
		FSSDSignalArray MultiplexedSamples[2];
		[unroll(2)]
			for (uint PairUncompressId = 0; PairUncompressId < 2; PairUncompressId++)
			{
				MultiplexedSamples[PairUncompressId] = UncompressMultiplexedSignals(
					KernelConfig, SampleBufferUV[PairUncompressId], CompressedMultiplexedSamples[PairUncompressId]);
			}

		// Take the min inverse frequency per signal if desired.
		// TODO(Denoiser): this should be normalized in theory...
		if (KernelConfig.bMinSamplePairInvFrequency)
		{
			[unroll(SIGNAL_ARRAY_SIZE)]
				for (uint SignalMultiplexId = 0; SignalMultiplexId < SIGNAL_ARRAY_SIZE; SignalMultiplexId++)
				{
					float MinInvFrequency = min(
						MultiplexedSamples[0].Array[SignalMultiplexId].WorldBluringRadius,
						MultiplexedSamples[1].Array[SignalMultiplexId].WorldBluringRadius);

					[flatten]
						if (MinInvFrequency > 0)
						{
							MultiplexedSamples[0].Array[SignalMultiplexId].WorldBluringRadius = MinInvFrequency;
							MultiplexedSamples[1].Array[SignalMultiplexId].WorldBluringRadius = MinInvFrequency;
						}
				}
		}

		FSSDSampleSceneInfos RefSceneMetadata = UncompressRefSceneMetadata(KernelConfig);

		[unroll(2)]
			for (uint PairAccumulateId = 0; PairAccumulateId < 2; PairAccumulateId++)
			{
				FSSDSampleSceneInfos SampleSceneMetadata = UncompressSampleSceneMetadata(
					KernelConfig, SampleBufferUV[PairAccumulateId], CompressedSampleSceneMetadata[PairAccumulateId]);

				AccumulateSampledMultiplexedSignals(
					KernelConfig,
					/* inout */ UncompressedAccumulators,
					/* inout */ CompressedAccumulators,
					RefSceneMetadata,
					SampleBufferUV[PairAccumulateId],
					SampleSceneMetadata,
					MultiplexedSamples[PairAccumulateId],
					KernelSampleWeight,
					/* bForceSample = */ false,
					bIsOutsideFrustum[PairAccumulateId]);
			}
	}
} // SampleAndAccumulateMultiplexedSignalsPair()

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

	[unroll(SIGNAL_ARRAY_SIZE)]
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

void DijestAccumulatedClusterSamples(
	inout FSSDSignalAccumulatorArray UncompressedAccumulators,
	inout FSSDCompressedSignalAccumulatorArray CompressedAccumulators,
	uint RingId, uint SampleCount)
{
#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION != ACCUMULATOR_COMPRESSION_DISABLED
	FSSDSignalAccumulatorArray Accumulators = UncompressAccumulatorArray(CompressedAccumulators, CONFIG_ACCUMULATOR_VGPR_COMPRESSION);
#endif

	[unroll(SIGNAL_ARRAY_SIZE)]
		for (uint SignalMultiplexId = 0; SignalMultiplexId < SIGNAL_ARRAY_SIZE; SignalMultiplexId++)
		{
#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION == ACCUMULATOR_COMPRESSION_DISABLED
			{
				DijestAccumulatedClusterSamples(
					/* inout */ UncompressedAccumulators.Array[SignalMultiplexId],
					RingId, SampleCount);
			}
#else
			{
				DijestAccumulatedClusterSamples(
					/* inout */ Accumulators.Array[SignalMultiplexId],
					RingId, SampleCount);
			}
#endif
		}

#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION != ACCUMULATOR_COMPRESSION_DISABLED
	CompressedAccumulators = CompressAccumulatorArray(Accumulators, CONFIG_ACCUMULATOR_VGPR_COMPRESSION);
#endif
}

void SampleAndAccumulateCenterSampleAsItsOwnCluster(
	FSSDKernelConfig KernelConfig,
	FSSDTexture2D SignalBuffer0,
	FSSDTexture2D SignalBuffer1,
	FSSDTexture2D SignalBuffer2,
	FSSDTexture2D SignalBuffer3,
	inout FSSDSignalAccumulatorArray UncompressedAccumulators,
	inout FSSDCompressedSignalAccumulatorArray CompressedAccumulators)
{
	const uint RingId = 0;

	FSSDSampleClusterInfo ClusterInfo;
	ClusterInfo.OutterBoundaryRadius = (RingId + 1) * KernelConfig.KernelSpreadFactor;

	StartAccumulatingCluster(
		KernelConfig,
		/* inout */ UncompressedAccumulators,
		/* inout */ CompressedAccumulators,
		ClusterInfo);

	SampleAndAccumulateMultiplexedSignals(
		KernelConfig,
		SignalBuffer0,
		SignalBuffer1,
		SignalBuffer2,
		SignalBuffer3,
		/* inout */ UncompressedAccumulators,
		/* inout */ CompressedAccumulators,
		KernelConfig.BufferUV,
		/* MipLevel = */ 0.0,
		/* KernelSampleWeight = */ 1.0,
		/* bForceSample = */ KernelConfig.bForceKernelCenterAccumulation);

	DijestAccumulatedClusterSamples(
		/* inout */ UncompressedAccumulators,
		/* inout */ CompressedAccumulators,
		RingId, /* SampleCount = */ 1);
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
				[unroll(2)]
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

								[branch]
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
		[loop]
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