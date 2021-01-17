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
#endif`

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

#endif
