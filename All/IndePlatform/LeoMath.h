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
#include "platform.h"
#include "leoint.hpp"
#include "memory.hpp"
#include <immintrin.h>
#include "leo_math_convert_impl.h"


//Macro
namespace leo
{
#undef _MM_SHUFFLE
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | \
                                     ((fp1) << 2) | ((fp0)))

#define LM_PERMUTE_PS( v, c ) _mm_shuffle_ps( v, v, c )
}

//Inhert std::
namespace leo
{
	using std::max;
	using std::min;
}

//Math Constant Number
namespace leo
{
	const float LM_PI = 3.14159265f;
	const float LM_HALFPI = LM_PI / 2.0f;
	const float LM_QUARPI = LM_PI / 4.0f;
	//radian per degree
	const float LM_RPD = LM_PI / 180.0f;
}

//Trigonometry Function<uint Radian>
namespace leo
{
#if LB_IMPL_MSCPP && PLATFORM_64BIT
#error "Unsuppot MSVC X64"
#else
	//from LH_MOUSE
	inline __declspec(naked) float __stdcall sincosr(float *pcos, float rad)
	{
		__asm
		{
			mov eax, dword ptr[esp + 4]
				fld dword ptr[esp + 8]
				fsincos
				fstp dword ptr[eax]
				ret 8
		}
	}

	inline float acosr(float r){
		float asm_one = 1.f;
		float asm_half_pi = LM_HALFPI;
		__asm {
			fld r // r0 = r
				fld r // r1 = r0, r0 = r
				fmul r // r0 = r0 * r
				fsubr asm_one // r0 = r0 - 1.f
				fsqrt // r0 = sqrtf( r0 )
				fchs // r0 = - r0
				fdiv // r0 = r1 / r0
				fld1 // {{ r0 = atan( r0 )
				fpatan // }}
				fadd asm_half_pi // r0 = r0 + pi / 2
		} // returns r0
	}

	inline float asinr(float r){
		const float asm_one = 1.f;
		__asm {
			fld r // r0 = r
				fld r // r1 = r0, r0 = r
				fmul r // r0 = r0 * r
				fsubr asm_one // r0 = r0 - 1.f
				fsqrt // r0 = sqrtf( r0 )
				fdiv // r0 = r1 / r0
				fld1 // {{ r0 = atan( r0 )
				fpatan // }}
		} // returns r0
	}

	inline float atanr(float r){
		__asm {
			fld r // r0 = r
				fld1 // {{ r0 = atan( r0 )
				fpatan // }}
		} // returns r0
	}

	inline float sinr(float r){
		__asm{
			fld r // r0 = r
				fsin // r0 = sinf(r0)
		}//return r0
	}

	inline float cosr(float r){
		__asm{
			fld r// r0 =r
				fcos// r0 = cosf(r0)
		}//returns r0
	}

	inline float tanr(float r){
		__asm{
			fld r
				fptan
		}
	}

	inline float sqrt(float f)
	{
		__asm{
			fld f
				fsqrt
		}
	}

	inline float rsqrt(float f){
		__asm{
			fld1
				fld f
				fsqrt
				fdiv
		}
	}
#endif
}



//Data Structure And Control Function
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

		float2(float X, float Y) lnoexcept()
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

		float2 max(const float2& rhs)  const lnoexcept()
		{
			return float2(leo::max(x, rhs.x), leo::max(y, rhs.y));
		}

		float2 min(const float2& rhs) const lnoexcept()
		{
			return float2(leo::min(x, rhs.x), leo::min(y, rhs.y));
		}
	};

	inline float2 max(const float2& lhs, const float2& rhs)
	{
		return lhs.max(rhs);
	}
	inline float2 min(const float2& lhs, const float2& rhs)
	{
		return lhs.min(rhs);
	}

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

		float3(float X, float Y, float Z) lnoexcept()
			:u(X), v(Y), w(Z)
		{}

		float3(const float2& XY, float Z) lnoexcept()
			: x(XY.x), y(XY.y), z(Z)
		{
		}

		float3(float X,const float2& YZ) lnoexcept()
			: x(X), y(YZ.x), z(YZ.y)
		{
		}

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
		float3 max(const float3& rhs)  const lnoexcept()
		{
			return float3(leo::max(x, rhs.x), leo::max(y, rhs.y), leo::max(z, rhs.z));
		}

		float3 min(const float3& rhs) const lnoexcept()
		{
			return float3(leo::min(x, rhs.x), leo::min(y, rhs.y), leo::min(z, rhs.z));
		}
	};

	inline float3 max(const float3& lhs, const float3& rhs)
	{
		return lhs.max(rhs);
	}
	inline float3 min(const float3& lhs, const float3& rhs)
	{
		return lhs.min(rhs);
	}

	struct lalignas(16) float4
	{
		union{
			struct{
				float x, y, z, w;
			};
			float data[4];
		};

		float4() lnoexcept() = default;

		float4(float X, float Y, float Z, float W) lnoexcept()
			:x(X), y(Y), z(Z), w(W)
		{}

		float4(const float2& XY, const float2& ZW) lnoexcept()
			: x(XY.x), y(XY.y), z(ZW.x), w(ZW.y)
		{
		}

		float4(const float2& XY,float Z,float W) lnoexcept()
			: x(XY.x), y(XY.y), z(Z), w(W)
		{
		}

		float4(float X,const float2& YZ, float W) lnoexcept()
			: x(X), y(YZ.x), z(YZ.y), w(W)
		{
		}

		float4(float X, float Y, const float2& ZW) lnoexcept()
			: x(X), y(Y), z(ZW.x), w(ZW.y)
		{
		}

		float4(const float3& XYZ, float W) lnoexcept()
			: x(XYZ.x), y(XYZ.y), z(XYZ.z), w(W)
		{
		}

		float4(float X, const float3& YZW) lnoexcept()
			: x(X), y(YZW.x), z(YZW.y), w(YZW.z)
		{
		}

		template<typename _Tx,typename _Ty>
		float4(const std::pair<_Tx, _Ty> XY, float Z, float W) lnoexcept()
			: x(XY.first), y(XY.second), z(Z), w(W)
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
		float4 max(const float4& rhs)  const lnoexcept()
		{
			return float4(leo::max(x, rhs.x), leo::max(y, rhs.y), leo::max(z, rhs.z), leo::max(w, rhs.w));
		}

		float4 min(const float4& rhs) const lnoexcept()
		{
			return float4(leo::min(x, rhs.x), leo::min(y, rhs.y), leo::min(z, rhs.z), leo::min(w, rhs.w));
		}
	};

	inline float4 max(const float4& lhs, const float4& rhs)
	{
		return lhs.max(rhs);
	}
	inline float4 min(const float4& lhs, const float4& rhs)
	{
		return lhs.min(rhs);
	}

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
			: half(float(i))
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

		half3(float X, float Y, float Z) lnoexcept()
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
		vst1_f32_ex( reinterpret_cast<float*>(&data), VL, 64 );
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
		vst1q_f32_ex( reinterpret_cast<float*>(&data),vector, 128 );
#elif defined(LM_SSE_INTRINSICS)
		_mm_store_ps(&data.x, vector);
#endif
	}
}

#pragma warning(push)
#pragma warning(disable : 4838)
#include <DirectXMath.h>
#include <DirectXCollision.h>
#pragma warning(pop)
namespace leo
{
	using namespace DirectX;
}

//Other Function
namespace leo
{


	template<typename _Ty>
	inline void clamp(const _Ty& _Min, const _Ty& _Max, _Ty & _X)
	{
		_X = leo::max(_Min, leo::min(_Max, _X));
	}

	inline float distance(const float2& left, const float2& right)
	{
#if 0
		auto x = load(left);
		auto y = load(right);
		auto V = x-y;
#if defined(LM_ARM_NEON_INTRINSICS)
		auto VL = vget_low_f32(V);
		// Dot2
		auto vTemp = vmul_f32(VL, VL);
		vTemp = vpadd_f32(vTemp, vTemp);
		const auto zero = vdup_n_f32(0);
		auto VEqualsZero = vceq_f32(vTemp, zero);
		// Sqrt
		auto S0 = vrsqrte_f32(vTemp);
		auto P0 = vmul_f32(vTemp, S0);
		auto R0 = vrsqrts_f32(P0, S0);
		auto S1 = vmul_f32(S0, R0);
		auto P1 = vmul_f32(vTemp, S1);
		auto R1 = vrsqrts_f32(P1, S1);
		auto Result = vmul_f32(S1, R1);
		Result = vmul_f32(vTemp, Result);
		Result = vbsl_f32(VEqualsZero, zero, Result);
		return vgetq_lane_f32(vcombine_f32(Result, Result),0);
#elif defined(LM_SSE_INTRINSICS)
		// Perform the dot product on x and y
		auto vLengthSq = _mm_mul_ps(V, V);
		// vTemp has y splatted
		auto vTemp = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(1, 1, 1, 1));
		// x+y
		vLengthSq = _mm_add_ss(vLengthSq, vTemp);
		vLengthSq = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(0, 0, 0, 0));
		vLengthSq = _mm_sqrt_ps(vLengthSq);
		return _mm_cvtss_f32(vLengthSq);
#endif
#else
		return leo::sqrt((left.x - right.x)*(left.x - right.x) + (left.y - right.y)*(left.y - right.y));
#endif
	}

	inline float distance(const float3& left, const float3& right)
	{
		auto x = load(left);
		auto y = load(right);
		auto V = x - y;
#if defined(LM_ARM_NEON_INTRINSICS)
		// Dot3
		float32x4_t vTemp = vmulq_f32(V, V);
		float32x2_t v1 = vget_low_f32(vTemp);
		float32x2_t v2 = vget_high_f32(vTemp);
		v1 = vpadd_f32(v1, v1);
		v2 = vdup_lane_f32(v2, 0);
		v1 = vadd_f32(v1, v2);
		const float32x2_t zero = vdup_n_f32(0);
		uint32x2_t VEqualsZero = vceq_f32(v1, zero);
		// Sqrt
		float32x2_t S0 = vrsqrte_f32(v1);
		float32x2_t P0 = vmul_f32(v1, S0);
		float32x2_t R0 = vrsqrts_f32(P0, S0);
		float32x2_t S1 = vmul_f32(S0, R0);
		float32x2_t P1 = vmul_f32(v1, S1);
		float32x2_t R1 = vrsqrts_f32(P1, S1);
		float32x2_t Result = vmul_f32(S1, R1);
		Result = vmul_f32(v1, Result);
		Result = vbsl_f32(VEqualsZero, zero, Result);
		return vgetq_lane_f32(vcombine_f32(Result, Result),0);
#elif defined(LM_SSE_INTRINSICS)
		// Perform the dot product on x,y and z
		__m128 vLengthSq = _mm_mul_ps(V, V);
		// vTemp has z and y
		__m128 vTemp = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(1, 2, 1, 2));
		// x+z, y
		vLengthSq = _mm_add_ss(vLengthSq, vTemp);
		// y,y,y,y
		vTemp = LM_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
		// x+z+y,??,??,??
		vLengthSq = _mm_add_ss(vLengthSq, vTemp);
		// Splat the length squared
		vLengthSq = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(0, 0, 0, 0));
		// Get the length
		vLengthSq = _mm_sqrt_ps(vLengthSq);
		return _mm_cvtss_f32(vLengthSq);
#endif
	}

	inline float distance(const float4& left, const float4& right)
	{
		auto x = load(left);
		auto y = load(right);
		auto V = x - y;
#if defined(LM_ARM_NEON_INTRINSICS)
		// Dot4
		float32x4_t vTemp = vmulq_f32(V, V);
		float32x2_t v1 = vget_low_f32(vTemp);
		float32x2_t v2 = vget_high_f32(vTemp);
		v1 = vpadd_f32(v1, v1);
		v2 = vpadd_f32(v2, v2);
		v1 = vadd_f32(v1, v2);
		const float32x2_t zero = vdup_n_f32(0);
		uint32x2_t VEqualsZero = vceq_f32(v1, zero);
		// Sqrt
		float32x2_t S0 = vrsqrte_f32(v1);
		float32x2_t P0 = vmul_f32(v1, S0);
		float32x2_t R0 = vrsqrts_f32(P0, S0);
		float32x2_t S1 = vmul_f32(S0, R0);
		float32x2_t P1 = vmul_f32(v1, S1);
		float32x2_t R1 = vrsqrts_f32(P1, S1);
		float32x2_t Result = vmul_f32(S1, R1);
		Result = vmul_f32(v1, Result);
		Result = vbsl_f32(VEqualsZero, zero, Result);
		return vgetq_lane_f32(vcombine_f32(Result, Result),0);
#elif defined(LM_SSE_INTRINSICS)
		// Perform the dot product on x,y,z and w
		XMVECTOR vLengthSq = _mm_mul_ps(V, V);
		// vTemp has z and w
		XMVECTOR vTemp = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(3, 2, 3, 2));
		// x+z, y+w
		vLengthSq = _mm_add_ps(vLengthSq, vTemp);
		// x+z,x+z,x+z,y+w
		vLengthSq = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(1, 0, 0, 0));
		// ??,??,y+w,y+w
		vTemp = _mm_shuffle_ps(vTemp, vLengthSq, _MM_SHUFFLE(3, 3, 0, 0));
		// ??,??,x+z+y+w,??
		vLengthSq = _mm_add_ps(vLengthSq, vTemp);
		// Splat the length
		vLengthSq = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(2, 2, 2, 2));
		// Prepare for the division
		vLengthSq = _mm_sqrt_ps(vLengthSq);
		return _mm_cvtss_f32(vLengthSq);
#endif
	}
}

#endif