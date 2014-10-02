////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/leomath.h
//  Version:     v1.00
//  Created:     9/28/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 要求目标处理器<X86支持SSE2指令集><X64支持MMX指令集>
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef IndePlatform_LeoMath_h
#define IndePlatform_LeoMath_h

#include "ldef.h"
#include "leoint.hpp"
#include "memory.hpp"
#include <immintrin.h>
#include "leo_math_convert_impl.h"

#if defined(_M_IX86) || defined(_M_X64)
#define LM_SSE_INTRINSICS
#elif defined(_M_ARM)
#define LM_ARM_NEON_INTRINSICS
#endif

namespace leo
{
	struct float2;
	struct float3;
	struct float4;
	struct half;
	struct half2;
	struct half3;
	struct half4;

	
	struct lalignas(16) float2
	{
		union{
			struct{
				float x, y;
			};
			struct{
				float u, v;
			};
			float data[2];
		};

		float2() lnoexcept() = default;

		float2(float X,float Y) lnoexcept()
			:u(X), v(Y)
		{}

		template<typename T>
		explicit float2(const T& src) lnoexcept()
		{
			static_assert(sizeof(T) >= sizeof(float2), "Need More Data");
			std::memcpy(this, &src, sizeof(float2));
		}

		template<typename T>
		float2& operator=(const T& src) lnoexcept()
		{
			static_assert(sizeof(T) >= sizeof(float2), "Need More Data");
			std::memcpy(this, &src, sizeof(float2));
			return *this;
		}

		template<typename T>
		T* operator&() lnoexcept()
		{
			static_assert(sizeof(float2) >= sizeof(T), "Data Don't Enough");
			return reinterpret_cast<T*>(this);
		}
	};

	struct lalignas(16) float3
	{
		union{
			struct{
				float x, y, z;
			};
			struct{
				float u, v, w;
			};
			float data[3];
		};

		float3() lnoexcept() = default;

		float3(float X, float Y,float Z) lnoexcept()
			:u(X), v(Y), w(Z)
		{}

		template<typename T>
		explicit float3(const T& src) lnoexcept()
		{
			static_assert(sizeof(T) >= sizeof(float3), "Need More Data");
			std::memcpy(this, &src, sizeof(float3));
		}

		template<typename T>
		float3& operator=(const T& src) lnoexcept()
		{
			static_assert(sizeof(T) >= sizeof(float3), "Need More Data");
			std::memcpy(this, &src, sizeof(float3));
			return *this;
		}

		template<typename T>
		T* operator &() lnoexcept()
		{
			static_assert(sizeof(float3) >= sizeof(T), "Data Don't Enough");
			return reinterpret_cast<T*>(this);
		}
	};

	struct lalignas(16) float4
	{
		union{
			struct{
				float x, y, z,w;
			};
			float data[4];
		};

		float4() lnoexcept() = default;

		float4(float X, float Y, float Z,float W) lnoexcept()
			:x(X), y(Y), z(Z), w(W)
		{}

		template<typename T>
		explicit float4(const T& src) lnoexcept()
		{
			static_assert(sizeof(T) >= sizeof(float4), "Need More Data");
			std::memcpy(this, &src, sizeof(float4));
		}

		template<typename T>
		float4& operator=(const T& src) lnoexcept()
		{
			static_assert(sizeof(T) >= sizeof(float4), "Need More Data");
			std::memcpy(this, &src, sizeof(float4));
			return *this;
		}

		template<typename T>
		T* operator &() lnoexcept()
		{
			static_assert(sizeof(float4) >= sizeof(T), "Data Don't Enough");
			return reinterpret_cast<T*>(this);
		}
	};

	//The HALF data type is equivalent to the IEEE 754 binary16 format
	//consisting of a sign bit, a 5-bit biased exponent, and a 10-bit mantissa.
	//[15] SEEEEEMMMMMMMMMM [0]
	//(-1) ^ S * 0.0, if E == 0 and M == 0,
	//(-1) ^ S * 2 ^ -14 * (M / 2 ^ 10), if E == 0 and M != 0,
	//(-1) ^ S * 2 ^ (E - 15) * (1 + M / 2 ^ 10), if 0 < E < 31,
	//(-1) ^ S * INF, if E == 31 and M == 0, or
	//NaN, if E == 31 and M != 0,
	struct lalignas(2) half
	{
		uint16 data;
		explicit half(float f) lnoexcept()
			:data(details::float_to_half(f))
		{
			
		}

		explicit half(int16 i) lnoexcept()
			:half(float(i))
		{
		}

		half& operator=(float f) lnoexcept()
		{
			*this = half(f);
		}

		half& operator=(int16 i) lnoexcept()
		{
			*this = half(i);
		}

		explicit operator float() const lnoexcept()
		{
			return details::half_to_float(data);
		}

		explicit operator int16() const lnoexcept()
		{
			return static_cast<int16>(details::half_to_float(data));
		}
	};

	struct lalignas(4) half2
	{
		union{
			struct{
				uint16 x, y;
			};
			struct{
				uint16 u, v;
			};
			uint16 data[2];
		};

		half2(float X, float Y) lnoexcept()
			:u(half(X).data), v(half(Y).data)
		{}

		half2()
		{}
	};

	struct half3
	{
		union{
			struct{
				uint16 x, y, z;
			};
			struct{
				uint16 u, v, w;
			};
			uint16 data[2];
		};

		half3(float X, float Y,float Z) lnoexcept()
			:u(half(X).data), v(half(Y).data), w(half(Z).data)
		{}
	};

	struct lalignas(8) half4
	{
		union{
			struct{
				uint16 x, y, z, w;
			};
			uint16 data[4];
		};

		half4(float X, float Y, float Z, float W) lnoexcept()
			:x(half(X).data), y(half(Y).data), z(half(Z).data), w(half(W).data)
		{}
	};

	template<typename T>
	__m128 inline load(const T& data)
	{
		return load<T>(data);
	}

	template<>
	__m128 inline load<float2>(const float2& data)
	{
#ifdef LM_ARM_NEON_INTRINSICS
		float32x2_t x = vld1_f32_ex( reinterpret_cast<const float*>(pSource), 64 );
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
		float32x4_t V = vld1q_f32_ex( reinterpret_cast<const float*>(&data), 128 );
		return vsetq_lane_f32(0, V, 3);
#elif defined(LM_SSE_INTRINSICS)
		union
		{
			uint32_t i[4];
			__m128 v;
	} g_XMMask3 = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };
		__m128 V = _mm_load_ps(&data.x);
		return _mm_and_ps(V, g_XMMask3.v);
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

	template<typename T>
	void inline save(T& data, __m128 vector)
	{
		return save<T>(data,vector);
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


#undef _MM_SHUFFLE
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | \
                                     ((fp1) << 2) | ((fp0)))

	template<>
	void inline save<float3>(float3& data, __m128 vector)
	{
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x2_t VL = vget_low_f32(vector);
		vst1_f32_ex( reinterpret_cast<float*>(&data), VL, 64 );
		vst1q_lane_f32(reinterpret_cast<float*>(&data) + 2, vector, 2);
#elif defined(LM_SSE_INTRINSICS)
		__m128 T = _mm_shuffle_ps(vector,vector, _MM_SHUFFLE(2, 2, 2, 2));
		_mm_storel_epi64(reinterpret_cast<__m128i*>(&data), _mm_castps_si128(vector));
		_mm_store_ss(&data.z, T);
#endif
	}

	template<>
	void  inline save<float4>(float4& data, __m128 vector)
	{
#if defined(LM_ARM_NEON_INTRINSICS)
		vst1q_f32_ex( reinterpret_cast<float*>(&data),vector, 128 );
#elif defined(LM_SSE_INTRINSICS)
		_mm_store_ps(&data.x,vector);
#endif
	}
}
#endif