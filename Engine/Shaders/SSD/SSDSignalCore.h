#ifndef SSDSignalCore_h
#define SSDSignalCore_h 1

#ifndef COMPILE_SIGNAL_COLOR
#define COMPILE_SIGNAL_COLOR 0
#endif

#ifndef COMPILE_SIGNAL_COLOR_SH
#define COMPILE_SIGNAL_COLOR_SH 0
#endif

#ifndef COMPILE_SIGNAL_COLOR_ARRAY
#define COMPILE_SIGNAL_COLOR_ARRAY 0
#endif

// Maximum number of indepedendent signal domain that can be denoised at same time.
#ifndef MAX_SIGNAL_BATCH_SIZE
#error Need to specify the number of signal domain being denoised.
#endif

// Number of sample that can be multiplexed into FSSDSignalSample.
#ifndef SIGNAL_ARRAY_SIZE
#error Need to specify the size of the signal array.
#endif

//------------------------------------------------------- STRUCTURE

/** Generic data structure to manipulate any kind of signal. */
struct FSSDSignalSample
{
	// Number of sample accumulated.
	float SampleCount;

	// Scene color and alpha.
#if COMPILE_SIGNAL_COLOR
	float4 SceneColor;
#endif

	// Array of colors.
#if COMPILE_SIGNAL_COLOR_ARRAY
	float3 ColorArray[COMPILE_SIGNAL_COLOR_ARRAY];
#endif

	// Spherical harmonic of the color.
#if COMPILE_SIGNAL_COLOR_SH
	FSSDSphericalHarmonicRGB ColorSH;
#endif

	// Number of ray that did not find any intersections.
	float MissCount;

	// Hit distance of a this sample, valid only if SampleCount == 1.
	float ClosestHitDistance;

	// Blur radius allowed for this sample, valid only if SampleCount == 1.
	float WorldBluringRadius;

	float TransmissionDistance;
};

/** Array of signal samples. Technically should be a typedef, but there is a bug in HLSL with the out operator of an array. */
struct FSSDSignalArray
{
	FSSDSignalSample Array[SIGNAL_ARRAY_SIZE];
};

FSSDSignalSample CreateSignalSampleFromScalarValue(float Scalar)
{
	FSSDSignalSample OutSample;
#if COMPILE_SIGNAL_COLOR
	OutSample.SceneColor = Scalar;
#endif
#if COMPILE_SIGNAL_COLOR_ARRAY
	{
		UNROLL_N(COMPILE_SIGNAL_COLOR_ARRAY)
			for (uint ColorId = 0; ColorId < COMPILE_SIGNAL_COLOR_ARRAY; ColorId++)
				OutSample.ColorArray[ColorId] = Scalar;
	}
#endif
#if COMPILE_SIGNAL_COLOR_SH && SPHERICAL_HARMONIC_ORDER == 2
	OutSample.ColorSH.R.V = Scalar;
	OutSample.ColorSH.G.V = Scalar;
	OutSample.ColorSH.B.V = Scalar;
#elif COMPILE_SIGNAL_COLOR_SH && SPHERICAL_HARMONIC_ORDER == 3
	OutSample.ColorSH.R.V0 = Scalar;
	OutSample.ColorSH.R.V1 = Scalar;
	OutSample.ColorSH.R.V2 = Scalar;
	OutSample.ColorSH.G.V0 = Scalar;
	OutSample.ColorSH.G.V1 = Scalar;
	OutSample.ColorSH.G.V2 = Scalar;
	OutSample.ColorSH.B.V0 = Scalar;
	OutSample.ColorSH.B.V1 = Scalar;
	OutSample.ColorSH.B.V2 = Scalar;
#endif
	OutSample.SampleCount = Scalar;
	OutSample.MissCount = Scalar;
	OutSample.ClosestHitDistance = Scalar;
	OutSample.WorldBluringRadius = Scalar;
	OutSample.TransmissionDistance = Scalar;
	return OutSample;
}

FSSDSignalSample CreateSignalSample()
{
	return CreateSignalSampleFromScalarValue(0.0);
}

FSSDSignalArray CreateSignalArrayFromScalarValue(float Scalar)
{
	FSSDSignalArray OutSamples;
	[unroll(SIGNAL_ARRAY_SIZE)]
		for (uint BatchedSignalId = 0; BatchedSignalId < SIGNAL_ARRAY_SIZE; BatchedSignalId++)
		{
			OutSamples.Array[BatchedSignalId] = CreateSignalSampleFromScalarValue(Scalar);
		}
	return OutSamples;
}

#endif