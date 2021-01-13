#ifndef SSDSignalAccumulatorArray_h
#define SSDSignalAccumulatorArray_h 1

#define MAX_ACCUMULATOR_COMPRESSED_VGPRS 24

#include "SSD/SSDSignalAccumulator.h"

struct FSSDSignalAccumulatorArray
{
	FSSDSignalAccumulator Array[SIGNAL_ARRAY_SIZE];
};

/** Created an initialised accumulator array. */
FSSDSignalAccumulatorArray CreateSignalAccumulatorArray()
{
	FSSDSignalAccumulatorArray Accumulators;

	[unroll(SIGNAL_ARRAY_SIZE)]
		for (uint i = 0; i < SIGNAL_ARRAY_SIZE; i++)
		{
			Accumulators.Array[i] = CreateSignalAccumulator();
		}
	return Accumulators;
}

#endif