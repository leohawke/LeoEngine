#include <immintrin.h>
#include <array>

//SplatVectorScalar
namespace leo {
	inline __m128 SplatZ(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vdupq_lane_f32(vget_high_f32(v), 0);
#elif defined(LM_SSE_INTRINSICS)
		return LM_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
#endif
	}

	inline __m128 SplatY(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vdupq_lane_f32(vget_low_f32(v), 1);
#elif defined(LM_SSE_INTRINSICS)
		return LM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
#endif
	}

	inline __m128 SplatX(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vdupq_lane_f32(vget_low_f32(v), 0);
#elif defined(LM_SSE_INTRINSICS)
		return LM_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
#endif
	}

	inline __m128 SplatW(__m128 v) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vdupq_lane_f32(vget_high_f32(v), 1);
#elif defined(LM_SSE_INTRINSICS)
		return LM_PERMUTE_PS(v, _MM_SHUFFLE(3, 3, 3, 3));
#endif
	}

}

//min,max
namespace leo{
	inline __m128 max(__m128 V1, __m128 V2) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vmaxq_f32(V1, V2);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_max_ps(V1, V2);
#else
#endif
	}

	inline __m128 min(__m128 V1, __m128 V2) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vminq_f32(V1, V2);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_min_ps(V1, V2);
#else 
#endif
	}
}


//+-*/
namespace leo {
	inline __m128 LM_VECTOR_CALL Add(__m128 al, __m128 ar) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vaddq_f32(al, ar);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_add_ps(al, ar);
#endif
	}

	inline __m128 Subtract(__m128 sl, __m128 sr) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vsubq_f32(sl, sr);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_sub_ps(sl, sr);
#endif
	}

	inline __m128 LM_VECTOR_CALL MultiplyAdd(__m128 ml, __m128 mr, __m128 ar) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vmlaq_f32(ar, ml, mr);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_add_ps(_mm_mul_ps(ml, mr), ar);
#endif
	}

	inline __m128 LM_VECTOR_CALL Multiply(__m128 ml, __m128 mr) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vmulq_f32(ml, mr);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_mul_ps(ml, mr);
#endif
	}

	inline __m128 LM_VECTOR_CALL Multiply(__m128 ml, float mr) {
		return Multiply(ml, Splat(mr));
	}

	inline std::array<__m128, 4> Multiply(const std::array<__m128, 4>& M1, const std::array<__m128, 4>& M2) {
#if defined(LM_SSE_INTRINSICS)
		std::array<__m128, 4> mResult;
		// Use vW to hold the original row
		auto vW = M1[0];
		// Splat the component X,Y,Z then W
		auto vX = LM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
		auto vY = LM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
		auto vZ = LM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
		vW = LM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
		// Perform the operation on the first row
		vX = _mm_mul_ps(vX, M2[0]);
		vY = _mm_mul_ps(vY, M2[1]);
		vZ = _mm_mul_ps(vZ, M2[2]);
		vW = _mm_mul_ps(vW, M2[3]);
		// Perform a binary add to reduce cumulative errors
		vX = _mm_add_ps(vX, vZ);
		vY = _mm_add_ps(vY, vW);
		vX = _mm_add_ps(vX, vY);
		mResult[0] = vX;
		// Repeat for the other 3 rows
		vW = M1[1];
		vX = LM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
		vY = LM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
		vZ = LM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
		vW = LM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
		vX = _mm_mul_ps(vX, M2[0]);
		vY = _mm_mul_ps(vY, M2[1]);
		vZ = _mm_mul_ps(vZ, M2[2]);
		vW = _mm_mul_ps(vW, M2[3]);
		vX = _mm_add_ps(vX, vZ);
		vY = _mm_add_ps(vY, vW);
		vX = _mm_add_ps(vX, vY);
		mResult[1] = vX;
		vW = M1[2];
		vX = LM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
		vY = LM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
		vZ = LM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
		vW = LM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
		vX = _mm_mul_ps(vX, M2[0]);
		vY = _mm_mul_ps(vY, M2[1]);
		vZ = _mm_mul_ps(vZ, M2[2]);
		vW = _mm_mul_ps(vW, M2[3]);
		vX = _mm_add_ps(vX, vZ);
		vY = _mm_add_ps(vY, vW);
		vX = _mm_add_ps(vX, vY);
		mResult[2] = vX;
		vW = M1[3];
		vX = LM_PERMUTE_PS(vW, _MM_SHUFFLE(0, 0, 0, 0));
		vY = LM_PERMUTE_PS(vW, _MM_SHUFFLE(1, 1, 1, 1));
		vZ = LM_PERMUTE_PS(vW, _MM_SHUFFLE(2, 2, 2, 2));
		vW = LM_PERMUTE_PS(vW, _MM_SHUFFLE(3, 3, 3, 3));
		vX = _mm_mul_ps(vX, M2[0]);
		vY = _mm_mul_ps(vY, M2[1]);
		vZ = _mm_mul_ps(vZ, M2[2]);
		vW = _mm_mul_ps(vW, M2[3]);
		vX = _mm_add_ps(vX, vZ);
		vY = _mm_add_ps(vY, vW);
		vX = _mm_add_ps(vX, vY);
		mResult[3] = vX;
		return mResult;
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	inline __m128 LM_VECTOR_CALL Multiply(__m128 v, const std::array<__m128, 4>& m) {
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x2_t VL = vget_low_f32(v);
		LMVECTOR vResult = LM_VMULQ_LANE_F32(m[0], VL, 0); // X
		vResult = LM_VMLAQ_LANE_F32(vResult, m[1], VL, 1); // Y
		float32x2_t VH = vget_high_f32(v);
		vResult = LM_VMLAQ_LANE_F32(vResult, m[2], VH, 0); // Z
		return LM_VMLAQ_LANE_F32(vResult, m[3], VH, 1); // W
#elif defined(LM_SSE_INTRINSICS)
		// Splat x,y,z and w
		auto vTempX = LM_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
		auto vTempY = LM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
		auto vTempZ = LM_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
		auto vTempW = LM_PERMUTE_PS(v, _MM_SHUFFLE(3, 3, 3, 3));
		// Mul by the matrix
		vTempX = _mm_mul_ps(vTempX, m[0]);
		vTempY = _mm_mul_ps(vTempY, m[1]);
		vTempZ = _mm_mul_ps(vTempZ, m[2]);
		vTempW = _mm_mul_ps(vTempW, m[3]);
		// Add them all together
		vTempX = _mm_add_ps(vTempX, vTempY);
		vTempZ = _mm_add_ps(vTempZ, vTempW);
		vTempX = _mm_add_ps(vTempX, vTempZ);
		return vTempX;
#else
#endif
	}

	//1.Calculate an estimate for the reciprocal of the divisor (D): X0.
	//2.Compute successively more accurate estimates of the reciprocal: (X1...X1)
	//3.Compute the quotient by multiplying the dividend by the reciprocal of the divisor: Q = NXs.
	// find the reciprocal of D, it is necessary to find a function f(X) which has a zero at X=1/D
	//f(x) = 1/X - D 
	//see href : http://en.wikipedia.org/wiki/Division_algorithm
	inline __m128 LM_VECTOR_CALL Divide(__m128 dl, __m128 dr) {
#if defined(LM_ARM_NEON_INTRINSICS)
		// 2 iterations of Newton-Raphson refinement of reciprocal
		float32x4_t Reciprocal = vrecpeq_f32(dr);
		float32x4_t S = vrecpsq_f32(Reciprocal, dr);
		Reciprocal = vmulq_f32(S, Reciprocal);
		S = vrecpsq_f32(Reciprocal, dr);
		Reciprocal = vmulq_f32(S, Reciprocal);
		return vmulq_f32(dl, Reciprocal);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_div_ps(dl, dr);
#endif
	}

	inline __m128 LM_VECTOR_CALL Divide(__m128 dl, float dr) {
		return Divide(dl, Splat(dr));
	}


#ifdef LB_IMPL_MSCPP
		inline __m128  operator * (__m128 lhs, __m128 rhs) {
			return Multiply(lhs, rhs);
		}

		inline __m128  operator * (__m128 lhs, float rhs) {
			return Multiply(lhs, rhs);
		}

		inline __m128  operator * (__m128 lhs, const std::array<__m128, 4>& rhs) {
			return Multiply(lhs, rhs);
		}

		inline __m128  operator / (__m128 lhs, __m128 rhs) {
			return Divide(lhs, rhs);
		}

		inline __m128  operator / (__m128 lhs, float rhs) {
			return Divide(lhs, rhs);
		}

		inline __m128  operator - (__m128 lhs, __m128 rhs) {
			return Subtract(lhs, rhs);
		}

		inline __m128  operator - (__m128 lhs) {
#if defined(LM_ARM_NEON_INTRINSICS)
			return vnegq_f32(V);
#elif defined(LM_SSE_INTRINSICS)
			__m128 Z;

			Z = _mm_setzero_ps();

			return _mm_sub_ps(Z, lhs);
#else 
#endif
		}

		inline __m128  operator + (__m128 lhs, __m128 rhs) {
			return Add(lhs, rhs);
		}

#endif
}
