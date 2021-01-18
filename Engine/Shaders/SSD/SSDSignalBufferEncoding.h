#ifndef SSDSignalBufferEncoding_h
#define SSDSignalBufferEncoding_h 1

#include "SSD/SSDSignalCore.h"



/** Whether the color should be clamped when encoding signal. */
#define CONFIG_ENCODING_CLAMP_COLOR 1

/** Selects the type that should be used when sampling a buffer */
#ifndef CONFIG_SIGNAL_INPUT_TEXTURE_TYPE
#define CONFIG_SIGNAL_INPUT_TEXTURE_TYPE SIGNAL_TEXTURE_TYPE_FLOAT4
#endif

/** Selects the type that should be used when sampling a buffer */
#ifndef CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE
#define CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE SIGNAL_TEXTURE_TYPE_FLOAT4
#endif

#if CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_FLOAT4
#define FSSDRawSample float4
#define FSSDTexture2D Texture2D
#elif CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT1 || CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT2 || CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT3 || CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT4
#if CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT1
#define FSSDRawSample uint
#elif CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT2
#define FSSDRawSample uint2
#elif CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT3
#define FSSDRawSample uint3
#elif CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT4
#define FSSDRawSample uint4
#else
#error Unknown input type for a signal texture.
#endif
#define FSSDTexture2D Texture2D<FSSDRawSample>
#else
#error Unknown input type for a signal texture.
#endif

#if CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_FLOAT4
#define FSSDOutputRawSample float4
#elif CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT1
#define FSSDOutputRawSample uint
#elif CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT2
#define FSSDOutputRawSample uint2
#elif CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT3
#define FSSDOutputRawSample uint3
#elif CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT4
#define FSSDOutputRawSample uint4
#else
#error Unknown output type for a signal texture.
#endif

#define FSSDRWTexture2D RWTexture2D<FSSDOutputRawSample>

/** Raw data layout when sampling input texture of the denoiser. */
struct FSSDCompressedMultiplexedSample
{
	FSSDRawSample VGPRArray[MAX_MULTIPLEXED_TEXTURES];
};


/** Decode input signal sample from raw float. */
void DecodeMultiplexedSignalsFromFloat4(
	const uint SignalBufferLayout,
	const uint MultiplexedSampleId,
	const bool bNormalizeSample,
	float4 RawSample[MAX_MULTIPLEXED_TEXTURES],
	out FSSDSignalArray OutSamples)
{
	OutSamples = CreateSignalArrayFromScalarValue(0.0);

	if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_INJESTION_NSPP)
	{
		[unroll(MAX_SIGNAL_BATCH_SIZE)]
			for (uint BatchSignalId = 0; BatchSignalId < MAX_SIGNAL_BATCH_SIZE; BatchSignalId++)
			{
				uint MultiplexId = BatchSignalId;
				float4 Channels = RawSample[MultiplexId];

				// TODO(Denoiser): feed the actual number of sample.
				OutSamples.Array[MultiplexId].SampleCount = (Channels.g == -2.0 ? 0.0 : 1.0);
				OutSamples.Array[MultiplexId].MissCount = OutSamples.Array[MultiplexId].SampleCount * Channels.r;
				OutSamples.Array[MultiplexId].WorldBluringRadius = OutSamples.Array[MultiplexId].SampleCount * (Channels.g == -1 ? WORLD_RADIUS_MISS : Channels.g);
				OutSamples.Array[MultiplexId].TransmissionDistance = OutSamples.Array[MultiplexId].SampleCount * Channels.a;
			}
	}
}

void DecodeMultiplexedSignalsFromUint2(
	const uint SignalBufferLayout,
	const uint MultiplexedSampleId,
	const bool bNormalizeSample,
	uint2 RawSample[MAX_MULTIPLEXED_TEXTURES],
	out FSSDSignalArray OutSamples)
{
	OutSamples = CreateSignalArrayFromScalarValue(0.0);

	if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_INJESTION_NSPP)
	{
		[unroll(MAX_SIGNAL_BATCH_SIZE)]
			for (uint BatchSignalId = 0; BatchSignalId < MAX_SIGNAL_BATCH_SIZE; BatchSignalId++)
			{
				uint MultiplexId = BatchSignalId;
				uint EncodedData = MultiplexId % 2 ? RawSample[MultiplexId / 2].g : RawSample[MultiplexId / 2].r;

				float MissCountRatio = (EncodedData & 0xFF) / 255.0;
				float TransmissionDistanceRatio = ((EncodedData >> 8) & 0xFF) / 255.0;
				float WorldBluringRadius = f16tof32(EncodedData >> 16);
				float SampleCount = (WorldBluringRadius == -2.0 ? 0.0 : 1.0);

				float MissCount = SampleCount * MissCountRatio;
				float TransmissionDistance = TransmissionDistanceRatio * 5.0;
				WorldBluringRadius = SampleCount * (WorldBluringRadius == -1.0 ? WORLD_RADIUS_MISS : WorldBluringRadius);

				// TODO(Denoiser): feed the actual number of sample.
				OutSamples.Array[MultiplexId].SampleCount = SampleCount;
				OutSamples.Array[MultiplexId].MissCount = MissCount;
				OutSamples.Array[MultiplexId].WorldBluringRadius = WorldBluringRadius;
				OutSamples.Array[MultiplexId].TransmissionDistance = TransmissionDistance;
			}
	}
}

/** Encode output signal sample. */
void EncodeMultiplexedSignals(
	const uint SignalBufferLayout, const uint MultiplexCount,
	FSSDSignalSample Sample[SIGNAL_ARRAY_SIZE],
	out FSSDOutputRawSample OutRawSample[MAX_MULTIPLEXED_TEXTURES],
	out uint OutBufferCount)
#if CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_FLOAT4
{
	// Init all raw samples.
	[unroll(MAX_MULTIPLEXED_TEXTURES)]
		for (uint i = 0; i < MAX_MULTIPLEXED_TEXTURES; i++)
			OutRawSample[i] = 0;

	// Number of buffer the signal get encoded onto <= MAX_MULTIPLEXED_TEXTURES.
	OutBufferCount = 1;

	if (0)
	{
		// NOP
	}
#if 1
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_INJESTION_NSPP)
	{
		[unroll]
			for (uint MultiplexId = 0; MultiplexId < MultiplexCount; MultiplexId++)
			{
				float4 EncodedChannels = float4(
					Sample[MultiplexId].MissCount,
					Sample[MultiplexId].WorldBluringRadius,
					0.0,
					Sample[MultiplexId].TransmissionDistance);
				EncodedChannels *= SafeRcp(Sample[MultiplexId].SampleCount);

				if (Sample[MultiplexId].SampleCount == 0)
				{
					EncodedChannels.y = -2;
				}
				else if (Sample[MultiplexId].WorldBluringRadius == WORLD_RADIUS_MISS)
				{
					EncodedChannels.y = -1;
				}

				OutRawSample[MultiplexId] = EncodedChannels;
			}
		OutBufferCount = MultiplexCount;
	}
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_HISTORY || SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_RECONSTRUCTION)
	{
		[unroll]
			for (uint MultiplexId = 0; MultiplexId < MultiplexCount; MultiplexId++)
			{
				float NormalizationFactor = SafeRcp(Sample[MultiplexId].SampleCount);

				float NormalizedWorldBluringRadius = Sample[MultiplexId].WorldBluringRadius * NormalizationFactor;
				float NormalizedTransmissionDistance = Sample[MultiplexId].TransmissionDistance * NormalizationFactor;

				OutRawSample[MultiplexId] = float4(
					Sample[MultiplexId].MissCount * NormalizationFactor,
					Sample[MultiplexId].SampleCount,
					NormalizedWorldBluringRadius == WORLD_RADIUS_MISS ? -1 : NormalizedWorldBluringRadius,
					NormalizedTransmissionDistance);
			}
		OutBufferCount = MultiplexCount;
	}
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_REJECTION)
	{
		const uint BatchSignalCount = MultiplexCount;
		OutBufferCount = BatchSignalCount;

		[unroll]
			for (uint BatchSignalId = 0; BatchSignalId < BatchSignalCount; BatchSignalId++)
			{
				uint MultiplexId = BatchSignalId;

				float NormalizationFactor = SafeRcp(Sample[MultiplexId + 0].SampleCount);

				float NormalizedWorldBluringRadius = Sample[MultiplexId + 0].WorldBluringRadius * NormalizationFactor;

				// Samples are normalised when doing history preconvolution, so the number of is either 0 or 1 on both momments. Therefore
				// Sample[MultiplexId + 0].SampleCount == Sample[MultiplexId + 1].SampleCount.
				OutRawSample[BatchSignalId] = float4(
					Sample[MultiplexId].MissCount * NormalizationFactor,
					Sample[MultiplexId].SampleCount,
					NormalizedWorldBluringRadius == WORLD_RADIUS_MISS ? -1 : NormalizedWorldBluringRadius,
					Sample[MultiplexId].TransmissionDistance * NormalizationFactor);
			}
	}
#endif
#if COMPILE_SIGNAL_COLOR_ARRAY >= 2
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_POLYCHROMATIC_PENUMBRA_HARMONIC_RECONSTRUCTION)
	{
		// diffuse harmonic
		{
			const uint MultiplexId = 0;

			float NormalizationFactor = SafeRcp(Sample[MultiplexId].SampleCount);

			OutRawSample[0].rgb = Sample[MultiplexId].ColorArray[0] * NormalizationFactor;
			OutRawSample[0].a = Sample[MultiplexId].SampleCount;
			OutRawSample[1].rgb = Sample[MultiplexId].ColorArray[1] * NormalizationFactor;
		}

		// specular harmonic
		{
			const uint MultiplexId = 1;

			float NormalizationFactor = SafeRcp(Sample[1].SampleCount);

			OutRawSample[2].rgb = Sample[MultiplexId].ColorArray[0] * NormalizationFactor;
			OutRawSample[2].a = Sample[MultiplexId].SampleCount;
			OutRawSample[3].rgb = Sample[MultiplexId].ColorArray[1] * NormalizationFactor;
		}

		OutBufferCount = 4;
	}
#endif
#if COMPILE_SIGNAL_COLOR
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_POLYCHROMATIC_PENUMBRA_HISTORY)
	{
		// Encode diffuse harmonic
		{
			const uint MultiplexId = 0;

			float NormalizationFactor = SafeRcp(Sample[MultiplexId].SampleCount);

			OutRawSample[0].rgb = ClampColorForEncoding(Sample[MultiplexId].SceneColor.rgb * NormalizationFactor);
			OutRawSample[0].a = Sample[MultiplexId].SampleCount;
		}

		// Encode specular harmonic
		{
			const uint MultiplexId = 1;

			float NormalizationFactor = SafeRcp(Sample[MultiplexId].SampleCount);

			OutRawSample[1].rgb = ClampColorForEncoding(Sample[MultiplexId].SceneColor.rgb * NormalizationFactor);
			OutRawSample[1].a = Sample[MultiplexId].SampleCount;
		}

		OutBufferCount = 2;
	}
#endif
#if COMPILE_SIGNAL_COLOR
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_REFLECTIONS_REJECTION)
	{
		// Outputs scene color to be compatible with a SSR output.
		OutRawSample[0] = ClampColorForEncoding(Sample[0].SceneColor * (Sample[0].SampleCount > 0 ? rcp(Sample[0].SampleCount) : 0));
		OutRawSample[1].r = Sample[0].SampleCount;
		OutBufferCount = 2;

		if (MultiplexCount == 2)
		{
			OutRawSample[2] = ClampColorForEncoding(Sample[1].SceneColor * (Sample[1].SampleCount > 0 ? rcp(Sample[1].SampleCount) : 0));
			OutRawSample[1].g = Sample[1].SampleCount;
			OutBufferCount = 3;
		}
	}
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_REFLECTIONS_HISTORY)
	{
		float NormalizationFactor = SafeRcp(Sample[0].SampleCount);

		// Outputs scene color to be compatible with a SSR output.
		OutRawSample[0] = ClampColorForEncoding(Sample[0].SceneColor * NormalizationFactor);
		OutRawSample[1].r = Sample[0].SampleCount;
		OutBufferCount = 2;

		// Temporal analysis.
#if SIGNAL_ARRAY_SIZE >= 3
		if (MultiplexCount == 3)
		{
			OutRawSample[1].g = Sample[1].SceneColor.x * NormalizationFactor;
			OutRawSample[1].b = Sample[2].SceneColor.x * NormalizationFactor;
			OutBufferCount = 3;
		}
#endif
	}
#endif
#if 1
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_AO_REJECTION)
	{
		// Outputs number of ray miss in red to be compatible as a SSAO output.
		OutRawSample[0].r = Sample[0].SampleCount > 0 ? Sample[0].MissCount / Sample[0].SampleCount : 1.0;
		OutRawSample[0].g = Sample[0].SampleCount;

		if (MultiplexCount == 2)
		{
			OutRawSample[0].b = Sample[1].SampleCount > 0 ? Sample[1].MissCount / Sample[1].SampleCount : 1.0;
			OutRawSample[0].a = Sample[1].SampleCount;
		}
	}
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_AO_HISTORY)
	{
		// Outputs number of ray miss in red to be compatible as a SSAO output.
		OutRawSample[0].r = Sample[0].SampleCount > 0 ? Sample[0].MissCount / Sample[0].SampleCount : 1.0;
		OutRawSample[0].g = Sample[0].SampleCount;
	}
#endif
#if COMPILE_SIGNAL_COLOR
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_DIFFUSE_INDIRECT_AND_AO_RECONSTRUCTION)
	{
		float NormalizationFactor = SafeRcp(Sample[0].SampleCount);

		OutRawSample[0] = ClampColorForEncoding(Sample[0].SceneColor * NormalizationFactor);
		OutRawSample[1].r = Sample[0].SampleCount;
		OutBufferCount = 2;
	}
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_DIFFUSE_INDIRECT_AND_AO_HISTORY)
	{
		float NormalizationFactor = SafeRcp(Sample[0].SampleCount);

		OutRawSample[0] = ClampColorForEncoding(Sample[0].SceneColor * NormalizationFactor);
		OutRawSample[1].r = Sample[0].SampleCount;
		OutBufferCount = 2;

		// Temporal analysis.
#if SIGNAL_ARRAY_SIZE >= 3
		if (MultiplexCount == 3)
		{
			OutRawSample[1].g = Sample[1].SceneColor.x * NormalizationFactor;
			OutRawSample[1].b = Sample[2].SceneColor.x * NormalizationFactor;
		}
#endif
	}
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_SSGI_HISTORY_R11G11B10)
	{
		float NormalizationFactor = SafeRcp(Sample[0].SampleCount);

		OutRawSample[0].rgb = ClampColorForEncoding(Sample[0].SceneColor * NormalizationFactor).rgb;
		OutRawSample[1].g = saturate(Sample[0].SampleCount * rcp(64.0));
		OutRawSample[1].r = Sample[0].MissCount * NormalizationFactor;
		OutBufferCount = 2;
	}
#endif
} // EncodeMultiplexedSignals()
#elif CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT2
{
	// Init all raw samples.
	UNROLL_N(MAX_MULTIPLEXED_TEXTURES)
		for (uint i = 0; i < MAX_MULTIPLEXED_TEXTURES; i++)
			OutRawSample[i] = 0;

	// Number of buffer the signal get encoded onto <= MAX_MULTIPLEXED_TEXTURES.
	OutBufferCount = 1;

	if (0)
	{
	}
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_INJESTION_NSPP)
	{
		UNROLL
			for (uint MultiplexId = 0; MultiplexId < MultiplexCount; MultiplexId++)
			{
				float MissCount = Sample[MultiplexId].MissCount;
				float WorldBluringRadius = Sample[MultiplexId].WorldBluringRadius;
				float TransmissionDistance = Sample[MultiplexId].TransmissionDistance;
				float SampleCount = Sample[MultiplexId].SampleCount;
				if (SampleCount == 0)
				{
					WorldBluringRadius = -2.0;
				}
				else if (WorldBluringRadius == WORLD_RADIUS_MISS)
				{
					WorldBluringRadius = -1.0;
				}

				float MissCountRatio = MissCount * SafeRcp(SampleCount);
				float TransmissionDistanceRatio = TransmissionDistance / 5.0;

				uint EncodedData = MissCountRatio * 255;
				EncodedData |= (uint(TransmissionDistanceRatio * 255) << 8);
				EncodedData |= (f32tof16(WorldBluringRadius) << 16);

				if (MultiplexId % 2)
				{
					OutRawSample[MultiplexId / 2].g = EncodedData;
				}
				else
				{
					OutRawSample[MultiplexId / 2].r = EncodedData;
				}
			}
		OutBufferCount = (MultiplexCount + 1) / 2;
	}
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_HISTORY || SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_RECONSTRUCTION)
	{
		UNROLL
			for (uint MultiplexId = 0; MultiplexId < MultiplexCount; MultiplexId++)
			{
				float SampleCount = Sample[MultiplexId].SampleCount;
				float NormalizationFactor = rcp(SampleCount);

				float MissCount = Sample[MultiplexId].MissCount;
				float WorldBluringRadius = (Sample[MultiplexId].WorldBluringRadius == WORLD_RADIUS_MISS ? -1.0 : Sample[MultiplexId].WorldBluringRadius);
				float TransmissionDistance = Sample[MultiplexId].TransmissionDistance;

				uint2 EncodedData = 0;
				EncodedData.x = f32tof16(MissCount);
				EncodedData.x |= f32tof16(SampleCount) << 16;
				EncodedData.y = f32tof16(WorldBluringRadius);
				EncodedData.y |= asuint(f32tof16(TransmissionDistance)) << 16;

				OutRawSample[MultiplexId] = EncodedData;
			}
		OutBufferCount = MultiplexCount;
	}
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_PENUMBRA_REJECTION)
	{
		const uint BatchSignalCount = MultiplexCount / 2;
		OutBufferCount = BatchSignalCount;

		UNROLL
			for (uint BatchSignalId = 0; BatchSignalId < BatchSignalCount; BatchSignalId++)
			{
				uint MultiplexId = BatchSignalId * 2;

				float SampleCount = Sample[MultiplexId + 0].SampleCount;
				float NormalizationFactor = rcp(SampleCount);

				float MissCount0 = Sample[MultiplexId + 0].MissCount;
				float MissCount1 = Sample[MultiplexId + 1].MissCount;
				float WorldBluringRadius = (Sample[MultiplexId + 0].WorldBluringRadius == WORLD_RADIUS_MISS ? -1.0 : Sample[MultiplexId + 0].WorldBluringRadius);
				float TransmissionDistance = 0.0;

				uint2 EncodedData = 0;
				EncodedData.x = f32tof16(MissCount0);
				EncodedData.x |= f32tof16(MissCount1) << 16;
				EncodedData.y = f32tof16(SampleCount);
				EncodedData.y |= f32tof16(WorldBluringRadius) << 16;

				OutRawSample[BatchSignalId] = EncodedData;
			}
	}
#if COMPILE_SIGNAL_COLOR_SH
	else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_DIFFUSE_INDIRECT_HARMONIC)
	{
		const uint EncodeOptions = (CONFIG_ENCODING_CLAMP_COLOR ? SSD_ENCODE_CLAMP_COLOR : 0) | SSD_ENCODE_NORMALIZE;

		EncodeDiffuseSphericalHarmonicTexel(
			Sample[0].SampleCount,
			Sample[0].MissCount,
			Sample[0].ColorSH,
			EncodeOptions,
			/* out */ OutRawSample);

		OutBufferCount = 4;
	}
#endif
} // EncodeMultiplexedSignals()
#else
#error Unimplemented.
#endif

/** Sample the raw of multiple input signals that have been multiplexed. */
FSSDCompressedMultiplexedSample SampleCompressedMultiplexedSignals(
	FSSDTexture2D SignalBuffer0, FSSDTexture2D SignalBuffer1, FSSDTexture2D SignalBuffer2, FSSDTexture2D SignalBuffer3,
	SamplerState Sampler, float2 UV, float Level, uint2 PixelCoord)
{
	FSSDCompressedMultiplexedSample CompressedSample;

	// Isolate the texture fetches to force lattency hiding, to outsmart compilers that tries to
	// discard some of the texture fetches for instance when SampleCount == 0 in another texture.
	{
		#if CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_FLOAT4
		{
			CompressedSample.VGPRArray[0] = SignalBuffer0.SampleLevel(Sampler, UV, Level);
			CompressedSample.VGPRArray[1] = SignalBuffer1.SampleLevel(Sampler, UV, Level);
			CompressedSample.VGPRArray[2] = SignalBuffer2.SampleLevel(Sampler, UV, Level);
			CompressedSample.VGPRArray[3] = SignalBuffer3.SampleLevel(Sampler, UV, Level);
		}
		#elif CONFIG_SIGNAL_INPUT_TEXTURE_TYPE >= SIGNAL_TEXTURE_TYPE_UINT1 && CONFIG_SIGNAL_INPUT_TEXTURE_TYPE <= SIGNAL_TEXTURE_TYPE_UINT4
		{
		// TODO(Denoiser): Exposed the int3 instead as function parameter.
		int3 Coord = int3(PixelCoord >> uint(Level), int(Level));
		CompressedSample.VGPRArray[0] = SignalBuffer0.Load(Coord);
		CompressedSample.VGPRArray[1] = SignalBuffer1.Load(Coord);
		CompressedSample.VGPRArray[2] = SignalBuffer2.Load(Coord);
		CompressedSample.VGPRArray[3] = SignalBuffer3.Load(Coord);
	}
	#else
		#error Unimplemented.
	#endif
	}

	return CompressedSample;
}

/** Decode input signal sample from raw float. */
FSSDSignalArray DecodeMultiplexedSignals(
	const uint SignalBufferLayout,
	const uint MultiplexedSampleId,
	const bool bNormalizeSample,
	FSSDCompressedMultiplexedSample CompressedSample)
{
#if CONFIG_SIGNAL_INPUT_TEXTURE_TYPE == SIGNAL_TEXTURE_TYPE_UINT2
	{
		FSSDSignalArray MultiplexedSamples = CreateSignalArrayFromScalarValue(0.0);
		DecodeMultiplexedSignalsFromUint2(
			SignalBufferLayout, MultiplexedSampleId, bNormalizeSample,
			CompressedSample.VGPRArray, /* out */ MultiplexedSamples);

		if (0)
		{

		}
#if COMPILE_SIGNAL_COLOR_SH
		else if (SignalBufferLayout == SIGNAL_BUFFER_LAYOUT_DIFFUSE_INDIRECT_HARMONIC)
		{
			const uint DecodeOptions = bNormalizeSample ? SSD_DECODE_NORMALIZE : 0x0;

			DecodeDiffuseSphericalHarmonicTexel(
				CompressedSample.VGPRArray,
				DecodeOptions,
				/* out */ MultiplexedSamples.Array[0].SampleCount,
				/* out */ MultiplexedSamples.Array[0].MissCount,
				/* out */ MultiplexedSamples.Array[0].ColorSH);

			// TODO(Denoiser): SRV on RG F32 and manually decode with f16tof32(), going to reduce VGPR pressure between fetch and result.
		}
#endif
		return MultiplexedSamples;
	}
#endif
}

/** Sample multiple input signals that have been multiplexed. */
FSSDSignalArray SampleMultiplexedSignals(
	FSSDTexture2D SignalBuffer0, FSSDTexture2D SignalBuffer1, FSSDTexture2D SignalBuffer2, FSSDTexture2D SignalBuffer3,
	SamplerState Sampler,
	const uint SignalBufferLayout, const uint MultiplexedSampleId,
	const bool bNormalizeSample,
	float2 UV, float Level = 0)
{
	uint2 PixelCoord = BufferUVToBufferPixelCoord(UV);

	FSSDCompressedMultiplexedSample CompressedSample = SampleCompressedMultiplexedSignals(
		SignalBuffer0, SignalBuffer1, SignalBuffer2, SignalBuffer3,
		Sampler, UV, Level, PixelCoord);

	return DecodeMultiplexedSignals(
		SignalBufferLayout, MultiplexedSampleId, bNormalizeSample, CompressedSample);
}

/** Outputs multiplexed signal. */
void OutputMultiplexedSignal(
	FSSDRWTexture2D OutputSignalBuffer0,
	FSSDRWTexture2D OutputSignalBuffer1,
	FSSDRWTexture2D OutputSignalBuffer2,
	FSSDRWTexture2D OutputSignalBuffer3,
	const uint SignalBufferLayout, const uint MultiplexCount,
	const uint2 PixelPosition, FSSDSignalSample MultiplexedSamples[SIGNAL_ARRAY_SIZE])
{
	// Encode the output signal.
	FSSDOutputRawSample RawSample[MAX_MULTIPLEXED_TEXTURES];
	uint BufferCount;
	EncodeMultiplexedSignals(
		SignalBufferLayout, MultiplexCount,
		MultiplexedSamples,
		/* out */ RawSample, /* out */ BufferCount);

	// Output the raw encoded sample according to number of RT they requires.
	if (BufferCount >= 1)
		OutputSignalBuffer0[PixelPosition] = RawSample[0];
	if (BufferCount >= 2)
		OutputSignalBuffer1[PixelPosition] = RawSample[1];
	if (BufferCount >= 3)
		OutputSignalBuffer2[PixelPosition] = RawSample[2];
	if (BufferCount >= 4)
		OutputSignalBuffer3[PixelPosition] = RawSample[3];
}

#endif
