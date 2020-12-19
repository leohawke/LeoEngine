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

#endif