////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/leomathsseconvert.hpp
//  Version:     v1.00
//  Created:     5/9/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: sse数据转换操作的集合
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////


#ifndef IndePlatform_LeoMathSSEconvert_hpp
#define IndePlatform_LeoMathSSEconvert_hpp

#include "leomathtype.hpp"
#include <immintrin.h>
#include <array>

//Macro

#undef _MM_SHUFFLE
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | \
                                     ((fp1) << 2) | ((fp0)))

#define LM_PERMUTE_PS( v, c ) _mm_shuffle_ps( v, v, c )

#ifdef LM_ARM_NEON_INTRINSICS
#define LM_VMULQ_N_F32( a, b ) vmulq_n_f32( (a), (b) )
#define LM_VMLAQ_N_F32( a, b, c ) vmlaq_n_f32( (a), (b), (c) )
#define LM_VMULQ_LANE_F32( a, b, c ) vmulq_lane_f32( (a), (b), (c) )
#define LM_VMLAQ_LANE_F32( a, b, c, d ) vmlaq_lane_f32( (a), (b), (c), (d) )
#endif



//helper struct
namespace leo {
	struct lalignas(16) vectorf32 {
		union
		{
			float f[4];
			__m128 v;
		};

		inline operator __m128() const { return v; }

		inline operator __m128i() const { return _mm_castps_si128(v); }
	};


	struct lalignas(16) vectori32
	{
		union
		{
			uint32 i[4];
			__m128 v;
		};

		inline operator __m128() const { return v; }
	};
}

namespace leo {


	template<typename T>
	__m128 inline load(const T& data)
	{
		return load<T>(data);
	}

	template<>
	__m128 inline load<float2>(const float2& data)
	{
#ifdef LM_ARM_NEON_INTRINSICS
		float32x2_t x = vld1_f32_ex(reinterpret_cast<const float*>(pSource), 64);
		float32x2_t zero = vdup_n_f32(0);
		return vcombine_f32(x, zero);
#elif defined(LM_SSE_INTRINSICS)
		__m128i V = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&data));
		return _mm_castsi128_ps(V);
#endif
	}

	template<>
	__m128 inline load<float3>(const float3& data)
	{
#ifdef LM_ARM_NEON_INTRINSICS
		float32x4_t V = vld1q_f32_ex(reinterpret_cast<const float*>(&data), 128);
		return vsetq_lane_f32(0, V, 3);
#elif defined(LM_SSE_INTRINSICS)
		vectori32 g_LMMask3 = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };
		__m128 V = _mm_load_ps(&data.x);
		return _mm_and_ps(V, g_LMMask3.v);
#endif
	}

	template<>
	__m128 inline load<float4>(const float4& data)
	{
#ifdef LM_ARM_NEON_INTRINSICS
		return vld1q_f32_ex(reinterpret_cast<const float*>(&data), 128);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_load_ps(&data.x);
#endif
	}


	inline std::array<__m128, 4> load(const float4x4& data) {
		return std::array < __m128, 4 >
		{
			{load(data[0]), load(data[1]), load(data[2]), load(data[3])}
		};
	}

	template<typename T>
	void inline save(T& data, __m128 vector)
	{
		return save<T>(data, vector);
	}

	template<>
	void  inline save<float2>(float2& data, __m128 vector)
	{
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x2_t VL = vget_low_f32(vector);
		vst1_f32(reinterpret_cast<float*>(data), VL);
#elif defined(LM_SSE_INTRINSICS)
		_mm_storel_epi64(reinterpret_cast<__m128i*>(&data), _mm_castps_si128(vector));
#endif
	}

	template<>
	void inline save<float3>(float3& data, __m128 vector)
	{
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x2_t VL = vget_low_f32(vector);
		vst1_f32_ex(reinterpret_cast<float*>(&data), VL, 64);
		vst1q_lane_f32(reinterpret_cast<float*>(&data) + 2, vector, 2);
#elif defined(LM_SSE_INTRINSICS)
		__m128 T = _mm_shuffle_ps(vector, vector, _MM_SHUFFLE(2, 2, 2, 2));
		_mm_storel_epi64(reinterpret_cast<__m128i*>(&data), _mm_castps_si128(vector));
		_mm_store_ss(&data.z, T);
#endif
	}

	template<>
	void  inline save<float4>(float4& data, __m128 vector)
	{
#if defined(LM_ARM_NEON_INTRINSICS)
		vst1q_f32_ex(reinterpret_cast<float*>(&data), vector, 128);
#elif defined(LM_SSE_INTRINSICS)
		_mm_store_ps(&data.x, vector);
#endif
	}

	template<>
	void inline save<float>(float& data, __m128 vector) {
#if defined(LM_ARM_NEON_INTRINSICS)
		vst1q_lane_f32(pDestination, vector, 0);
#elif defined(LM_SSE_INTRINSICS)
		_mm_store_ss(&data, vector);
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	void inline save(float4x4& data, const std::array<__m128, 4>& M) {
#if defined(LM_SSE_INTRINSICS)
		_mm_store_ps(&data[0].x, M[0]);
		_mm_store_ps(&data[1].x, M[1]);
		_mm_store_ps(&data[2].x, M[2]);
		_mm_store_ps(&data[3].x, M[3]);
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	inline __m128 set(float x, float y, float z, float w) {
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x2_t V0 = vcreate_f32(((uint64_t)*(const uint32_t *)&x) | ((uint64_t)(*(const uint32_t *)&y) << 32));
		float32x2_t V1 = vcreate_f32(((uint64_t)*(const uint32_t *)&z) | ((uint64_t)(*(const uint32_t *)&w) << 32));
		return vcombine_f32(V0, V1);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_set_ps(w, z, y, x);
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS
	}


	const uint32 LM_SELECT_1 = 0xFFFFFFFF;
	const uint32 LM_SELECT_0 = 0x00000000;

	inline __m128 Select(__m128 V1, __m128 V2, __m128 Control) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vbslq_f32(Control, V2, V1);
#elif defined(LM_SSE_INTRINSICS)
		auto vTemp1 = _mm_andnot_ps(Control, V1);
		auto vTemp2 = _mm_and_ps(V2, Control);
		return _mm_or_ps(vTemp1, vTemp2);
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	inline __m128 Splat(float value) {
#if defined(LM_ARM_NEON_INTRINSICS)
		return vld1q_dup_f32(&value);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_load_ps1(&value);
#else
#endif
	}
}

#endif