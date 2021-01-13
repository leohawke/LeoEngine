#ifndef SSDSignalAccumulator_h
#define SSDSignalAccumulator_h 1

#include "SSD/SSDSignalCore.h"

#ifndef CONFIG_SIGNAL_VGPR_COMPRESSION
#define CONFIG_SIGNAL_VGPR_COMPRESSION SIGNAL_COMPRESSION_DISABLED
#endif

#ifndef COMPILE_MOMENT1_ACCUMULATOR
#define COMPILE_MOMENT1_ACCUMULATOR 0
#endif

#ifndef COMPILE_MOMENT2_ACCUMULATOR
#define COMPILE_MOMENT2_ACCUMULATOR 0
#endif

#ifndef COMPILE_MINMAX_ACCUMULATOR
#define COMPILE_MINMAX_ACCUMULATOR 0
#endif

#ifndef COMPILE_MININVFREQ_ACCUMULATOR
#define COMPILE_MININVFREQ_ACCUMULATOR 0
#endif

#ifndef COMPILE_DRB_ACCUMULATOR
#define COMPILE_DRB_ACCUMULATOR 0
#endif

/* LeoEngine will fork DirextShaderCompile to support template
*/
struct FSSDSignalAccumulator
{
	// Sums of moment.
#if COMPILE_MOMENT1_ACCUMULATOR
	FSSDSignalSample Moment1;
#endif

#if COMPILE_MOMENT2_ACCUMULATOR
	FSSDSignalSample Moment2;
#endif

	// Compressed form of moment signals.
#if COMPILE_MOMENT1_ACCUMULATOR
	FSSDCompressedSignalSample CompressedMoment1;
#endif

#if COMPILE_MOMENT2_ACCUMULATOR
	FSSDCompressedSignalSample CompressedMoment2;
#endif

	// Per component min and max values.
#if COMPILE_MINMAX_ACCUMULATOR
	FSSDSignalSample Min;
	FSSDSignalSample Max;
#endif

	// Minimal inverse frequency found and intersected when sampling.
#if COMPILE_MININVFREQ_ACCUMULATOR
	float MinInvFrequency;
#endif

	// Descending ring bucketing.
#if COMPILE_DRB_ACCUMULATOR
	FSSDSignalSample Current;
	float CurrentInvFrequency;
	float CurrentTranslucency;

	FSSDSignalSample Previous;
	float PreviousInvFrequency;

	float BorderingRadius;
	float HighestInvFrequency;
#endif // COMPILE_DRB_ACCUMULATOR
};

FSSDSignalAccumulator CreateSignalAccumulator()
{
	FSSDSignalAccumulator Accumulator;

#if COMPILE_MOMENT1_ACCUMULATOR
	Accumulator.Moment1 = CreateSignalSampleFromScalarValue(0.0);
#endif

#if COMPILE_MOMENT2_ACCUMULATOR
	Accumulator.Moment2 = CreateSignalSampleFromScalarValue(0.0);
#endif	

#if COMPILE_MOMENT1_ACCUMULATOR
	Accumulator.CompressedMoment1 = CreateCompressedSignalSampleFromScalarValue(0.0, CONFIG_SIGNAL_VGPR_COMPRESSION);
#endif

#if COMPILE_MOMENT2_ACCUMULATOR
	Accumulator.CompressedMoment2 = CreateCompressedSignalSampleFromScalarValue(0.0, CONFIG_SIGNAL_VGPR_COMPRESSION);
#endif

#if COMPILE_MINMAX_ACCUMULATOR
	Accumulator.Min = CreateSignalSampleFromScalarValue(INFINITE_FLOAT);
	Accumulator.Max = CreateSignalSampleFromScalarValue(0.0); // TODO(Denoiser): -INFINITE_FLOAT? otherwise there is a risk to color clamp with YCoCg.
#endif

#if COMPILE_MININVFREQ_ACCUMULATOR
	Accumulator.MinInvFrequency = WORLD_RADIUS_MISS;
#endif

	// DRB initialization.
#if COMPILE_DRB_ACCUMULATOR
	{
		Accumulator.Current = CreateSignalSampleFromScalarValue(0.0);
		Accumulator.CurrentInvFrequency = 0.0;
		Accumulator.CurrentTranslucency = 0.0;

		Accumulator.Previous = CreateSignalSampleFromScalarValue(0.0);
		Accumulator.PreviousInvFrequency = 0.0;

		Accumulator.BorderingRadius = 0.0;
		Accumulator.HighestInvFrequency = 0.0;
	}
#endif

	return Accumulator;
}


#endif
