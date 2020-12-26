#ifndef SSDSignalArray_h
#define SSDSignalArray_h 1

#include "SSD/SSDSignalCore.h"

FSSDSignalArray NormalizeToOneSampleArray(FSSDSignalArray Samples)
{
	FSSDSignalArray OutSamples;
	[unroll(SIGNAL_ARRAY_SIZE)]
		for (uint BatchedSignalId = 0; BatchedSignalId < SIGNAL_ARRAY_SIZE; BatchedSignalId++)
		{
			OutSamples.Array[BatchedSignalId] = NormalizeToOneSample(Samples.Array[BatchedSignalId]);
		}
	return OutSamples;
}

#endif