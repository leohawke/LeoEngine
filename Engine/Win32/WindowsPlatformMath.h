#pragma once

#include <LBase/linttype.hpp>
#include <intrin0.h>

namespace platform_ex::Windows
{
	using namespace leo::inttype;

	inline uint32 CountLeadingZeros(uint32 Value)
	{
		// Use BSR to return the log2 of the integer
		unsigned long Log2;
		if (_BitScanReverse(&Log2, Value) != 0)
		{
			return 31 - Log2;
		}

		return 32;
	}

	inline uint32 CeilLogTwo(uint32 Arg)
	{
		int32 Bitmask = ((int32)(CountLeadingZeros(Arg) << 26)) >> 31;
		return (32 - CountLeadingZeros(Arg - 1)) & (~Bitmask);
	}

	inline uint32 RoundUpToPowerOfTwo(uint32 Arg)
	{
		return 1 << CeilLogTwo(Arg);
	}

	template<typename _type>
	inline uint32 FloorLog2(_type Value)
	{
		unsigned long Log2;
		if (_BitScanReverse(&Log2, Value) != 0)
		{
			return Log2;
		}

		return 0;
	}

	template<>
	inline uint32 FloorLog2<uint64>(uint64 Value)
	{
		unsigned long Log2;
		if (_BitScanReverse64(&Log2, Value) != 0)
		{
			return Log2;
		}

		return 0;
	}
}
