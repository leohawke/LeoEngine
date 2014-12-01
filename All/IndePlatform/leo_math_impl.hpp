////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/leo_math_impl.hpp
//  Version:     v1.00
//  Created:     11/30/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 几何体
// -------------------------------------------------------------------------
//  History:
//		
//
////////////////////////////////////////////////////////////////////////////

#ifndef IndePlatform_LeoMath_h
#error "不要包含实现头文件"
#endif

#ifndef IndePlatform_LeoMath_impl_hpp
#define IndePlatform_LeoMath_impl_hpp

#include "leomath.h"

namespace leo {
	

	inline __m128 LM_VECTOR_CALL operator*(__m128 lhs, __m128 rhs)  {
		return Multiply(lhs, rhs);
	}

	inline __m128 LM_VECTOR_CALL operator-(__m128 lhs, __m128 rhs) {
		return Subtract(lhs, rhs);
	}

	inline __m128 LM_VECTOR_CALL operator+(__m128 lhs, __m128 rhs) {
		return Add(lhs, rhs);
	}

	inline float GetX(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vgetq_lane_f32(v, 0);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cvtss_f32(v);
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	inline float GetY(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vgetq_lane_f32(v, 0);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cvtss_f32(LM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1)));
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	inline float GetZ(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vgetq_lane_f32(v, 0);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cvtss_f32(LM_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2)));
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	inline float GetW(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vgetq_lane_f32(v, 0);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cvtss_f32(LM_PERMUTE_PS(v, _MM_SHUFFLE(3, 3, 3, 3)));
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	namespace details {
		inline bool QuaternionIsUnit(__m128 Q)
		{
			static const vectorf32 UintQuaternionEpsilon = { 1.0e-4f, 1.0e-4f, 1.0e-4f, 1.0e-4f };

			__m128 Difference = Length<4>(Q) - SplatOne();
			return Less(Abs(Difference), UintQuaternionEpsilon);
		}

		inline __m128 SplatR3() {
			const static float4	r3(0.0f, 0.0f, 0.0f, 1.0f);
			const static auto _mr1 = load(r3);
			return _mr1;
		}

	}

}

#endif
