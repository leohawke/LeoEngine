////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/leomath.h
//  Version:     v1.01
//  Created:     9/28/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 要求目标处理器<X86支持SSE2指令集><X64支持MMX指令集>
// -------------------------------------------------------------------------
//  History:
//			 10/21/2014 改动了头文件,新增了函数
////////////////////////////////////////////////////////////////////////////

#ifndef IndePlatform_LeoMath_h
#define IndePlatform_LeoMath_h

#include "ldef.h"
#include "platform_macro.h"
#include "leoint.hpp"
#include "memory.hpp"
#include <immintrin.h>
#include "leo_math_convert_impl.h"
#include <array>

//Macro
namespace leo
{
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
	const float LM_TWOPI = LM_PI*2.f;
	const float LM_HALFPI = LM_PI / 2.0f;
	const float LM_QUARPI = LM_PI / 4.0f;

	const float LM_1DIV2PI = 0.159154943f;
	//radian per degree
	const float LM_RPD = LM_PI / 180.0f;

	const std::uint8_t zero = 0;
	const std::uint8_t one = 1;
	const std::uint8_t two = 2;
	const std::uint8_t four = 4;
	const std::uint8_t eight = 8;
}

//Trigonometry Function<uint Radian>
namespace leo
{
#if LB_IMPL_MSCPP && !PLATFORM_64BIT
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
#else
	inline float acosr(float r){
		return std::acos(r);
	}

	inline float asinr(float r){
		return std::asin(r);
	}

	inline float atanr(float r){
		return std::atan(r);
	}

	inline float sinr(float r){
		return std::sin(r);
	}

	inline float cosr(float r){
		return std::cos(r);
	}

	inline float tanr(float r){
		return std::tan(r);
	}

	inline float sqrt(float f)
	{
		return std::sqrt(r);
	}

	inline float rsqrt(float f){
		return 1.f/sqrt(f);
	}

	inlien float sincosr(float *pcos, float rad){
		*pcos = cosr(rad);
		return sinr(rad);
	}
#endif

	inline float atanr(float x, float y)
	{
		float theta = 0.f;
		if (x >= 0.f)
		{
			theta = atanr(y / x);

			if (theta < 0.f)
				theta += 2.0f*LM_PI;
		}
		else
			theta = atanr(y / x) + LM_PI;
		return theta;
	}
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

		float2() lnothrow = default;

		float2(float X, float Y) lnothrow
			:u(X), v(Y)
		{}

		template<typename T>
		explicit float2(const T& src) lnothrow
		{
			static_assert(sizeof(T) >= sizeof(float2), "Need More Data");
			std::memcpy(this, &src, sizeof(float2));
		}

			template<typename T>
		float2& operator=(const T& src) lnothrow
		{
			static_assert(sizeof(T) >= sizeof(float2), "Need More Data");
			std::memcpy(this, &src, sizeof(float2));
			return *this;
		}

			template<typename T>
		T* operator&() lnothrow
		{
			static_assert(sizeof(float2) >= sizeof(T), "Data Don't Enough");
			return reinterpret_cast<T*>(this);
		}

			float2 max(const float2& rhs)  const lnothrow
		{
			return float2(leo::max(x, rhs.x), leo::max(y, rhs.y));
		}

			float2 min(const float2& rhs) const lnothrow
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

		float3() lnothrow = default;

		float3(float X, float Y, float Z) lnothrow
			:u(X), v(Y), w(Z)
		{}

		float3(const float2& XY, float Z) lnothrow
			: x(XY.x), y(XY.y), z(Z)
		{
		}

		float3(float X, const float2& YZ) lnothrow
			: x(X), y(YZ.x), z(YZ.y)
		{
		}

		template<typename T>
		explicit float3(const T& src) lnothrow
		{
			static_assert(sizeof(T) >= sizeof(float3), "Need More Data");
			std::memcpy(this, &src, sizeof(float3));
		}

			template<typename T>
		float3& operator=(const T& src) lnothrow
		{
			static_assert(sizeof(T) >= sizeof(float3), "Need More Data");
			std::memcpy(this, &src, sizeof(float3));
			return *this;
		}

			template<typename T>
		T* operator &() lnothrow
		{
			static_assert(sizeof(float3) >= sizeof(T), "Data Don't Enough");
			return reinterpret_cast<T*>(this);
		}
			float3 max(const float3& rhs)  const lnothrow
		{
			return float3(leo::max(x, rhs.x), leo::max(y, rhs.y), leo::max(z, rhs.z));
		}

			float3 min(const float3& rhs) const lnothrow
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

		float4() lnothrow = default;

		float4(float X, float Y, float Z, float W) lnothrow
			:x(X), y(Y), z(Z), w(W)
		{}

		float4(const float2& XY, const float2& ZW) lnothrow
			: x(XY.x), y(XY.y), z(ZW.x), w(ZW.y)
		{
		}

		float4(const float2& XY, float Z, float W) lnothrow
			: x(XY.x), y(XY.y), z(Z), w(W)
		{
		}

		float4(float X, const float2& YZ, float W) lnothrow
			: x(X), y(YZ.x), z(YZ.y), w(W)
		{
		}

		float4(float X, float Y, const float2& ZW) lnothrow
			: x(X), y(Y), z(ZW.x), w(ZW.y)
		{
		}

		float4(const float3& XYZ, float W) lnothrow
			: x(XYZ.x), y(XYZ.y), z(XYZ.z), w(W)
		{
		}

		float4(float X, const float3& YZW) lnothrow
			: x(X), y(YZW.x), z(YZW.y), w(YZW.z)
		{
		}

		template<typename _Tx, typename _Ty>
		float4(const std::pair<_Tx, _Ty> XY, float Z, float W) lnothrow
			: x(XY.first), y(XY.second), z(Z), w(W)
		{}

		template<typename T>
		explicit float4(const T& src) lnothrow
		{
			static_assert(sizeof(T) >= sizeof(float4), "Need More Data");
			std::memcpy(this, &src, sizeof(float4));
		}

			template<typename T>
		float4& operator=(const T& src) lnothrow
		{
			static_assert(sizeof(T) >= sizeof(float4), "Need More Data");
			std::memcpy(this, &src, sizeof(float4));
			return *this;
		}



			template<typename T>
		T* operator &() lnothrow
		{
			static_assert(sizeof(float4) >= sizeof(T), "Data Don't Enough");
			return reinterpret_cast<T*>(this);
		}
			float4 max(const float4& rhs)  const lnothrow
		{
			return float4(leo::max(x, rhs.x), leo::max(y, rhs.y), leo::max(z, rhs.z), leo::max(w, rhs.w));
		}

			float4 min(const float4& rhs) const lnothrow
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

	struct lalignas(16) float4x4{
		float4 r[4];

		float& operator()(uint8 row, uint8 col){
			return r[row].data[col];
		}

		float operator()(uint8 row, uint8 col) const{
			return r[row].data[col];
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
		explicit half(float f) lnothrow
			:data(details::float_to_half(f))
		{

		}

		explicit half(int16 i) lnothrow
			: half(float(i))
		{
		}

		half& operator=(float f) lnothrow
		{
			*this = half(f);
		}

			half& operator=(int16 i) lnothrow
		{
			*this = half(i);
		}

			explicit operator float() const lnothrow
		{
			return details::half_to_float(data);
		}

			explicit operator int16() const lnothrow
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

		half2(float X, float Y) lnothrow
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

		half3(float X, float Y, float Z) lnothrow
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

		half4(float X, float Y, float Z, float W) lnothrow
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
		} g_LMMask3 = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };
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

	inline std::array<__m128, 4> load(const float4x4& data){
		return std::array < __m128, 4 >
		{
			{load(data.r[0]), load(data.r[1]), load(data.r[2]), load(data.r[3])}
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

	template<>
	void inline save<float>(float& data, __m128 vector){
#if defined(LM_ARM_NEON_INTRINSICS)
		vst1q_lane_f32(pDestination, vector, 0);
#elif defined(LM_SSE_INTRINSICS)
		_mm_store_ss(&data, vector);
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	void inline save(float4x4& data, const std::array<__m128, 4>& M){
#if defined(LM_SSE_INTRINSICS)
		_mm_store_ps(&data.r[0].x, M[0]);
		_mm_store_ps(&data.r[1].x, M[1]);
		_mm_store_ps(&data.r[2].x, M[2]);
		_mm_store_ps(&data.r[3].x, M[3]);
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	inline __m128 Select(__m128 V1, __m128 V2, __m128 Control){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vbslq_f32(Control, V2, V1);
#elif defined(LM_SSE_INTRINSICS)
		auto vTemp1 = _mm_andnot_ps(Control, V1);
		auto vTemp2 = _mm_and_ps(V2, Control);
		return _mm_or_ps(vTemp1, vTemp2);
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}
}

//details
namespace leo{
	namespace details{
		inline __m128 SplatEpsilon(){
			const static float4 eps(1.0e-4f, 1.0e-4f, 1.0e-4f, 1.0e-4f);
			const static auto _meps = load(eps);
			return _meps;
		}

		inline __m128 SplatRayEpsilon(){
			const static float4 eps(1.0e-20f, 1.0e-20f, 1.0e-20f, 1.0e-20f);
			const static auto _meps = load(eps);
			return _meps;
		}

		inline __m128 SplatNegRayEpsilon(){
			const static float4 eps(-1.0e-20f, -1.0e-20f, -1.0e-20f, -1.0e-20f);
			const static auto _meps = load(eps);
			return _meps;
		}

		template<uint8 D = 3>
		inline bool IsUnit(__m128 V){
			auto Difference = Subtract(Length<>(V), SplatOne());
			return Less<>(Abs(Difference), SplatEpsilon());
		}

		inline __m128 SplatInfinity(){
			const static lalignas(16) uint32 infinity[4] = { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 };
			const static auto _minfinity = load(*reinterpret_cast<const float4 *>(infinity));
			return _minfinity;
		}

		inline __m128 SplatQNaN(){
			const static lalignas(16) uint32 qnan[4] = { 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 };
			const static auto _mqnan = load(*reinterpret_cast<const float4 *>(qnan));
			return _mqnan;
		}

		inline __m128 SplatMask3(){
			const static lalignas(16) uint32 mask3[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };
			const static auto _mmask3 = load(*reinterpret_cast<const float4 *>(mask3));
			return _mmask3;
		}

		inline __m128 SplatTrueInt(){
#if defined(LM_ARM_NEON_INTRINSICS)
			return vdupq_n_s32(-1);
#elif defined(LM_SSE_INTRINSICS)
			__m128i V = _mm_set1_epi32(-1);
			return _mm_castsi128_ps(V);
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
		}

		inline __m128 SplatAbsMask(){
			const static lalignas(16) uint32 absmask[4] = { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF };
			const static auto _mabsmask = load(*reinterpret_cast<const float4 *>(absmask));
			return _mabsmask;
		}

		inline __m128 IsInfinite(__m128 V){
#if defined(LM_ARM_NEON_INTRINSICS)
			// Mask off the sign bit
			uint32x4_t vTemp = vandq_u32(V, g_XMAbsMask);
			// Compare to infinity
			vTemp = vceqq_f32(vTemp, g_XMInfinity);
			// If any are infinity, the signs are true.
			return vTemp;
#elif defined(LM_SSE_INTRINSICS)
			// Mask off the sign bit
			__m128 vTemp = _mm_and_ps(V, SplatAbsMask());
			// Compare to infinity
			vTemp = _mm_cmpeq_ps(vTemp, SplatInfinity());
			// If any are infinity, the signs are true.
			return vTemp;
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
		}

		inline __m128 SplatNegativeZero(){
			const static lalignas(16) uint32 zero[4] = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
			const static auto _mzero = load(*reinterpret_cast<const float4 *>(zero));
			return _mzero;
		}

		inline __m128 SplatR0(){
			const static float4	r0(1.0f, 0.0f, 0.0f, 0.0f);
			const static auto _mr0 = load(r0);
			return _mr0;
		}

	}
}

//__m128,std::arrry<__m128,4> operator def
namespace leo{
	inline __m128 SplatZ(__m128 v){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vdupq_lane_f32(vget_high_f32(v), 0);
#elif defined(LM_SSE_INTRINSICS)
		return LM_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
#endif
	}

	inline __m128 SplatY(__m128 v){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vdupq_lane_f32(vget_low_f32(v), 1);
#elif defined(LM_SSE_INTRINSICS)
		return LM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
#endif
	}

	inline __m128 SplatX(__m128 v){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vdupq_lane_f32(vget_low_f32(v), 0);
#elif defined(LM_SSE_INTRINSICS)
		return LM_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
#endif
	}

	inline __m128 SplatW(__m128 v){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vdupq_lane_f32(vget_high_f32(v),1);
#elif defined(LM_SSE_INTRINSICS)
		return LM_PERMUTE_PS(v, _MM_SHUFFLE(3, 3, 3, 3));
#endif
	}

	inline __m128 SplatOne(){
		const static float4 one(1.f, 1.f, 1.f, 1.f);
		const static auto _mone = load(one);
		return _mone;
	}

	inline __m128 SplatZero(){
		const static float4 zero(0.f, 0.f, 0.f, 0.f);
		const static auto _mzero = load(zero);
		return _mzero;
	}

	inline __m128 SplatNegativeOne(){
		const static float4 one(-1.f, -1.f, -1.f, -1.f);
		const static auto _mone = load(one);
		return _mone;
	}

	inline __m128 SplatHalfPi(){
		const static float4 halfpi(LM_HALFPI, LM_HALFPI, LM_HALFPI, LM_HALFPI);
		const static auto _mone = load(halfpi);
		return _mone;
	}

	inline __m128 SplatPi(){
		const static float4 pi(LM_PI, LM_PI, LM_PI, LM_PI);
		const static auto _mpi = load(pi);
		return _mpi;
	}

	inline __m128 SplatTwoPi(){
		const static float4 twopi(LM_TWOPI, LM_TWOPI, LM_TWOPI, LM_TWOPI);
		const static auto _mtwo = load(twopi);
		return _mtwo;
	}

	inline __m128 SplatReciprocalTwoPi(){
		const static float4 rtwopi(LM_1DIV2PI, LM_1DIV2PI, LM_1DIV2PI, LM_1DIV2PI);
		const static auto _mrtwopi = load(rtwopi);
		return _mrtwopi;
	}

	inline __m128 Splat(float value){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vld1q_dup_f32(&value);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_load_ps1(&value);
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	inline __m128 Subtract(__m128 sl, __m128 sr){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vsubq_f32(sl,sr);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_sub_ps(sl, sr);
#endif
	}

	inline __m128 Add(__m128 al, __m128 ar){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vaddq_f32(al,ar);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_add_ps(al, ar);
#endif
	}

	inline __m128 MultiplyAdd(__m128 ml, __m128 mr, __m128 ar){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vmlaq_f32(ar,ml,mr);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_add_ps(_mm_mul_ps(ml, mr), ar);
#endif
	}

	inline __m128 Multiply(__m128 ml, __m128 mr){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vmulq_f32(ml, mr);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_mul_ps(ml, mr);
#endif
	}

	inline std::array<__m128, 4> Multiply(const std::array<__m128, 4>& M1, const std::array<__m128, 4>& M2){
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

	//1.Calculate an estimate for the reciprocal of the divisor (D): X0.
	//2.Compute successively more accurate estimates of the reciprocal: (X1...X1)
	//3.Compute the quotient by multiplying the dividend by the reciprocal of the divisor: Q = NXs.
	// find the reciprocal of D, it is necessary to find a function f(X) which has a zero at X=1/D
	//f(x) = 1/X - D 
	//see href : http://en.wikipedia.org/wiki/Division_algorithm
	inline __m128 Divide(__m128 dl, __m128 dr){
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

	template<uint8 D = 3>
	inline __m128 TransformCoord(__m128 v, const std::array<__m128, 4>& m){
#if defined(LM_SSE_INTRINSICS) || defined(LM_ARM_NEON_INTRINSICS)
		auto z = SplatZ(v);
		auto y = SplatY(v);
		auto x = SplatX(v);

		auto result = MultiplyAdd(z, m[2], m[3]);
		result = MultiplyAdd(y, m[1], result);
		result = MultiplyAdd(x, m[0], result);

		auto w = SplatW(result);

		return Divide(result, w);
#endif
	}

	template<>
	inline __m128 TransformCoord<2>(__m128 v, const std::array<__m128, 4>& m){
#if defined(LM_SSE_INTRINSICS) || defined(LM_ARM_NEON_INTRINSICS)
		auto y = SplatY(v);
		auto x = SplatX(v);

		auto result = MultiplyAdd(y, m[1], m[3]);
		result = MultiplyAdd(x, m[0], result);

		auto w = SplatW(result);

		return Divide(result, w);
#endif
	}

	template<uint8 D = 3>
	inline __m128 TransformNormal(__m128 v, const std::array<__m128, 4>& m){
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x2_t VL = vget_low_f32(v);
		auto vResult = LM_VMULQ_LANE_F32(m[0], VL, 0); // X
		vResult = LM_VMLAQ_LANE_F32(vResult, m[1], VL, 1); // Y
		return LM_VMLAQ_LANE_F32(vResult, m[2], vget_high_f32(V), 0); // Z
#elif defined(LM_SSE_INTRINSICS)
		auto vResult = LM_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
		vResult = _mm_mul_ps(vResult, m[0]);
		auto vTemp = LM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
		vTemp = _mm_mul_ps(vTemp, m[1]);
		vResult = _mm_add_ps(vResult, vTemp);
		vTemp = LM_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
		vTemp = _mm_mul_ps(vTemp, m[2]);
		vResult = _mm_add_ps(vResult, vTemp);
		return vResult;
#else // _LM_VMX128_INTRINSICS_
#endif
	}

	template<>
	inline __m128 TransformNormal<2>(__m128 v, const std::array<__m128, 4>& m){
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x2_t VL = vget_low_f32(v);
		float32x4_t Result = LM_VMULQ_LANE_F32(m[1], VL, 1); // Y
		return LM_VMLAQ_LANE_F32(Result, m[0], VL, 0); // X
#elif defined(LM_SSE_INTRINSICS)
		auto vResult = LM_PERMUTE_PS(v, _MM_SHUFFLE(0, 0, 0, 0));
		vResult = _mm_mul_ps(vResult, m[0]);
		auto vTemp = LM_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
		vTemp = _mm_mul_ps(vTemp, m[1]);
		vResult = _mm_add_ps(vResult, vTemp);
		return vResult;
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	inline __m128 Transform(__m128 v, const std::array<__m128, 4>& m){
#if defined(_LM_ARM_NEON_INTRINSICS)
		float32x2_t VL = vget_low_f32(v);
		LMVECTOR vResult = LM_VMULQ_LANE_F32(m[0], VL, 0); // X
		vResult = LM_VMLAQ_LANE_F32(vResult,m[1], VL, 1); // Y
		float32x2_t VH = vget_high_f32(v);
		vResult = LM_VMLAQ_LANE_F32(vResult, m[2], VH, 0); // Z
		return LM_VMLAQ_LANE_F32(vResult,m[3], VH, 1); // W
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
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	template<uint8 D = 3>
	inline __m128 Normalize(__m128 V){
#if defined(LM_ARM_NEON_INTRINSICS)
		// Dot3
		float32x4_t vTemp = vmulq_f32(V, V);
		float32x2_t v1 = vget_low_f32(vTemp);
		float32x2_t v2 = vget_high_f32(vTemp);
		v1 = vpadd_f32(v1, v1);
		v2 = vdup_lane_f32(v2, 0);
		v1 = vadd_f32(v1, v2);
		uint32x2_t VEqualsZero = vceq_f32(v1, vdup_n_f32(0));
		uint32x2_t VEqualsInf = vceq_f32(v1, vget_low_f32(gLMInfinity));
		// Reciprocal sqrt (2 iterations of Newton-Raphson)
		float32x2_t S0 = vrsqrte_f32(v1);
		float32x2_t P0 = vmul_f32(v1, S0);
		float32x2_t R0 = vrsqrts_f32(P0, S0);
		float32x2_t S1 = vmul_f32(S0, R0);
		float32x2_t P1 = vmul_f32(v1, S1);
		float32x2_t R1 = vrsqrts_f32(P1, S1);
		v2 = vmul_f32(S1, R1);
		// Normalize
		auto vResult = vmulq_f32(V, vcombine_f32(v2, v2));
		vResult = vbslq_f32(vcombine_f32(VEqualsZero, VEqualsZero), vdupq_n_f32(0), vResult);
		return vbslq_f32(vcombine_f32(VEqualsInf, VEqualsInf), gLMQNaN, vResult);
#elif defined(LM_SSE_INTRINSICS)
		// Perform the dot product on x,y and z only
		auto vLengthSq = _mm_mul_ps(V, V);
		auto vTemp = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(2, 1, 2, 1));
		vLengthSq = _mm_add_ss(vLengthSq, vTemp);
		vTemp = LM_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
		vLengthSq = _mm_add_ss(vLengthSq, vTemp);
		vLengthSq = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(0, 0, 0, 0));
		// Prepare for the division
		auto vResult = _mm_sqrt_ps(vLengthSq);
		// Create zero with a single instruction
		auto vZeroMask = _mm_setzero_ps();
		// Test for a divide by zero (Must be FP to detect -0.0)
		vZeroMask = _mm_cmpneq_ps(vZeroMask, vResult);
		// Failsafe on zero (Or epsilon) length planes
		// If the length is infinity, set the elements to zero
		vLengthSq = _mm_cmpneq_ps(vLengthSq, details::SplatInfinity());
		// Divide to perform the normalization
		vResult = _mm_div_ps(V, vResult);
		// Any that are infinity, set to zero
		vResult = _mm_and_ps(vResult, vZeroMask);
		// Select qnan or result based on infinite length
		auto vTemp1 = _mm_andnot_ps(vLengthSq, details::SplatQNaN());
		auto vTemp2 = _mm_and_ps(vResult, vLengthSq);
		vResult = _mm_or_ps(vTemp1, vTemp2);
		return vResult;
#endif // _LM_VMX128_INTRINSICS_
	}

	template<>
	inline __m128 Normalize<2>(__m128 V){
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x2_t VL = vget_low_f32(V);
		// Dot2
		float32x2_t vTemp = vmul_f32(VL, VL);
		vTemp = vpadd_f32(vTemp, vTemp);
		uint32x2_t VEqualsZero = vceq_f32(vTemp, vdup_n_f32(0));
		uint32x2_t VEqualsInf = vceq_f32(vTemp, vget_low_f32(gLMInfinity));
		// Reciprocal sqrt (2 iterations of Newton-Raphson)
		float32x2_t S0 = vrsqrte_f32(vTemp);
		float32x2_t P0 = vmul_f32(vTemp, S0);
		float32x2_t R0 = vrsqrts_f32(P0, S0);
		float32x2_t S1 = vmul_f32(S0, R0);
		float32x2_t P1 = vmul_f32(vTemp, S1);
		float32x2_t R1 = vrsqrts_f32(P1, S1);
		vTemp = vmul_f32(S1, R1);
		// Normalize
		float32x2_t Result = vmul_f32(VL, vTemp);
		Result = vbsl_f32(VEqualsZero, vdup_n_f32(0), Result);
		Result = vbsl_f32(VEqualsInf, vget_low_f32(gLMQNaN), Result);
		return vcombine_f32(Result, Result);
#elif defined(LM_SSE_INTRINSICS)
		// Perform the dot product on x and y only
		auto vLengthSq = _mm_mul_ps(V, V);
		auto vTemp = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(1, 1, 1, 1));
		vLengthSq = _mm_add_ss(vLengthSq, vTemp);
		vLengthSq = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(0, 0, 0, 0));
		// Prepare for the division
		auto vResult = _mm_sqrt_ps(vLengthSq);
		// Create zero with a single instruction
		auto vZeroMask = _mm_setzero_ps();
		// Test for a divide by zero (Must be FP to detect -0.0)
		vZeroMask = _mm_cmpneq_ps(vZeroMask, vResult);
		// Failsafe on zero (Or epsilon) length planes
		// If the length is infinity, set the elements to zero
		vLengthSq = _mm_cmpneq_ps(vLengthSq, details::SplatInfinity());
		// Reciprocal mul to perform the normalization
		vResult = _mm_div_ps(V, vResult);
		// Any that are infinity, set to zero
		vResult = _mm_and_ps(vResult, vZeroMask);
		// Select qnan or result based on infinite length
		auto vTemp1 = _mm_andnot_ps(vLengthSq, details::SplatQNaN());
		auto vTemp2 = _mm_and_ps(vResult, vLengthSq);
		vResult = _mm_or_ps(vTemp1, vTemp2);
		return vResult;
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	template<uint8 D = 3>
	inline __m128 Length(__m128 V){
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
		return vcombine_f32(Result, Result);
#elif defined(LM_SSE_INTRINSICS)
		// Perform the dot product on x,y and z
		auto vLengthSq = _mm_mul_ps(V, V);
		// vTemp has z and y
		auto vTemp = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(1, 2, 1, 2));
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
		return vLengthSq;
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}



	inline __m128 Abs(__m128 V){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vabsq_f32(V);
#elif defined(LM_SSE_INTRINSICS)
		auto vResult = _mm_setzero_ps();
		vResult = _mm_sub_ps(vResult, V);
		vResult = _mm_max_ps(vResult, V);
		return vResult;
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	template<uint8 D = 3>
	inline __m128 Cross(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x2_t v1xy = vget_low_f32(V1);
		float32x2_t v2xy = vget_low_f32(V2);

		float32x2_t v1yx = vrev64_f32(v1xy);
		float32x2_t v2yx = vrev64_f32(v2xy);

		float32x2_t v1zz = vdup_lane_f32(vget_high_f32(V1), 0);
		float32x2_t v2zz = vdup_lane_f32(vget_high_f32(V2), 0);

		auto vResult = vmulq_f32(vcombine_f32(v1yx, v1xy), vcombine_f32(v2zz, v2yx));
		vResult = vmlsq_f32(vResult, vcombine_f32(v1zz, v1yx), vcombine_f32(v2yx, v2xy));
		vResult = veorq_u32(vResult, gLMFlipY);
		return vandq_u32(vResult, gLMMask3);
#elif defined(LM_SSE_INTRINSICS)
		// y1,z1,x1,w1
		auto vTemp1 = LM_PERMUTE_PS(V1, _MM_SHUFFLE(3, 0, 2, 1));
		// z2,x2,y2,w2
		auto vTemp2 = LM_PERMUTE_PS(V2, _MM_SHUFFLE(3, 1, 0, 2));
		// Perform the left operation
		auto vResult = _mm_mul_ps(vTemp1, vTemp2);
		// z1,x1,y1,w1
		vTemp1 = LM_PERMUTE_PS(vTemp1, _MM_SHUFFLE(3, 0, 2, 1));
		// y2,z2,x2,w2
		vTemp2 = LM_PERMUTE_PS(vTemp2, _MM_SHUFFLE(3, 1, 0, 2));
		// Perform the right operation
		vTemp1 = _mm_mul_ps(vTemp1, vTemp2);
		// Subract the right from left, and return answer
		vResult = _mm_sub_ps(vResult, vTemp1);
		// Set w to zero
		return _mm_and_ps(vResult, details::SplatMask3());
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	template<uint8 D = 3>
	inline __m128 Dot(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x4_t vTemp = vmulq_f32(V1, V2);
		float32x2_t v1 = vget_low_f32(vTemp);
		float32x2_t v2 = vget_high_f32(vTemp);
		v1 = vpadd_f32(v1, v1);
		v2 = vdup_lane_f32(v2, 0);
		v1 = vadd_f32(v1, v2);
		return vcombine_f32(v1, v1);
#elif defined(LM_SSE_INTRINSICS)
		// Perform the dot product
		auto vDot = _mm_mul_ps(V1, V2);
		// x=Dot.vector4_f32[1], y=Dot.vector4_f32[2]
		auto vTemp = LM_PERMUTE_PS(vDot, _MM_SHUFFLE(2, 1, 2, 1));
		// Result.vector4_f32[0] = x+y
		vDot = _mm_add_ss(vDot, vTemp);
		// x=Dot.vector4_f32[2]
		vTemp = LM_PERMUTE_PS(vTemp, _MM_SHUFFLE(1, 1, 1, 1));
		// Result.vector4_f32[0] = (x+y)+z
		vDot = _mm_add_ss(vDot, vTemp);
		// Splat x
		return LM_PERMUTE_PS(vDot, _MM_SHUFFLE(0, 0, 0, 0));
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	template<>
	inline __m128 Dot<4>(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x4_t vTemp = vmulq_f32(V1, V2);
		float32x2_t v1 = vget_low_f32(vTemp);
		float32x2_t v2 = vget_high_f32(vTemp);
		v1 = vpadd_f32( v1, v1 );
		v2 = vpadd_f32( v2, v2 );
		v1 = vadd_f32( v1, v2 );
		return vcombine_f32( v1, v1 );
#elif defined(LM_SSE_INTRINSICS)
		auto vTemp2 = V2;
		auto vTemp = _mm_mul_ps(V1, vTemp2);
		vTemp2 = _mm_shuffle_ps(vTemp2, vTemp, _MM_SHUFFLE(1, 0, 0, 0)); // Copy X to the Z position and Y to the W position
		vTemp2 = _mm_add_ps(vTemp2, vTemp);          // Add Z = X+Z; W = Y+W;
		vTemp = _mm_shuffle_ps(vTemp, vTemp2, _MM_SHUFFLE(0, 3, 0, 0));  // Copy W to the Z position
		vTemp = _mm_add_ps(vTemp, vTemp2);           // Add Z and W together
		return LM_PERMUTE_PS(vTemp, _MM_SHUFFLE(2, 2, 2, 2));    // Splat Z and return
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}

	inline __m128 Sqrt(__m128 V){
#if defined(LM_ARM_NEON_INTRINSICS)
		// 3 iterations of Newton-Raphson refinment of sqrt
		float32x4_t S0 = vrsqrteq_f32(V);
		float32x4_t P0 = vmulq_f32(V, S0);
		float32x4_t R0 = vrsqrtsq_f32(P0, S0);
		float32x4_t S1 = vmulq_f32(S0, R0);
		float32x4_t P1 = vmulq_f32(V, S1);
		float32x4_t R1 = vrsqrtsq_f32(P1, S1);
		float32x4_t S2 = vmulq_f32(S1, R1);
		float32x4_t P2 = vmulq_f32(V, S2);
		float32x4_t R2 = vrsqrtsq_f32(P2, S2);
		float32x4_t S3 = vmulq_f32(S2, R2);

		auto VEqualsInfinity = EqualInt(V, g_LMInfinity.v);
		auto VEqualsZero = Equal(V, vdupq_n_f32(0));
		auto Result = vmulq_f32(V, S3);
		auto Select = EqualInt(VEqualsInfinity, VEqualsZero);
		return Select(V, Result, Select);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_sqrt_ps(V);
#else // _LM_VMX128_INTRINSICS_
#endif // _LM_VMX128_INTRINSICS_
	}

	inline __m128 Round(__m128 V){
#if defined(LM_ARM_NEON_INTRINSICS)
		static const autoI32 magic = { 0x4B000000, 0x4B000000, 0x4B000000, 0x4B000000 };
		uint32x4_t sign = vandq_u32(V, g_XMNegativeZero);
		uint32x4_t sMagic = vorrq_u32(magic, sign);
		auto vResult = vaddq_f32(V, sMagic);
		vResult = vsubq_f32(vResult, sMagic);
		return vResult;
#elif defined(LM_SSE_INTRINSICS)
		static const lalignas(16) uint32 magic[] = { 0x4B000000, 0x4B000000, 0x4B000000, 0x4B000000 };
		__m128 sign = _mm_and_ps(V, details::SplatNegativeZero());
		__m128 sMagic = _mm_or_ps(load(*(float4*)(magic)), sign);
		auto vResult = _mm_add_ps(V, sMagic);
		vResult = _mm_sub_ps(vResult, sMagic);
		return vResult;
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}

	inline __m128 Lerp(__m128 V1, __m128 V2, __m128 T){
		return MultiplyAdd(V1, Subtract(SplatOne(), T), Multiply(V2, T));
	}
}

//__m128 logic operator def
namespace leo{
	template<uint8 D = 4>
	inline bool Less(__m128 lhs, __m128 rhs){
#if defined(LM_ARM_NEON_INTRINSICS)
		uint32x4_t vResult = vcltq_f32(V1, V2);
		int8x8x2_t vTemp = vzip_u8(vget_low_u8(vResult), vget_high_u8(vResult));
		vTemp = vzip_u16(vTemp.val[0], vTemp.val[1]);
		return (vget_lane_u32(vTemp.val[1], 1) == 0xFFFFFFFFU);
#elif defined(LM_SSE_INTRINSICS)
		auto vTemp = _mm_cmplt_ps(lhs, rhs);
		return ((_mm_movemask_ps(vTemp) == 0x0f) != 0);
#else
#endif
	}

	template<uint8 D = 3>
	inline bool GreaterOrEqual(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		uint32x4_t vResult = vcgeq_f32(V1, V2);
		int8x8x2_t vTemp = vzip_u8(vget_low_u8(vResult), vget_high_u8(vResult));
		vTemp = vzip_u16(vTemp.val[0], vTemp.val[1]);
		return ((vget_lane_u32(vTemp.val[1], 1) & 0xFFFFFFU) == 0xFFFFFFU);
#elif defined(LM_SSE_INTRINSICS)
		auto vTemp = _mm_cmpge_ps(V1, V2);
		return (((_mm_movemask_ps(vTemp) & 7) == 7) != 0);
#else // LM_VMX128_INTRINSICS_
#endif
	}

	template<uint8 D = 3>
	inline bool LessOrEqual(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		int32x4_t vResult = vcleq_f32( V1, V2 );
		int8x8x2_t vTemp = vzip_u8(vget_low_u8(vResult), vget_high_u8(vResult));
		vTemp = vzip_u16(vTemp.val[0], vTemp.val[1]);
		return ( (vget_lane_u32(vTemp.val[1], 1) & 0xFFFFFFU) == 0xFFFFFFU );
#elif defined(LM_SSE_INTRINSICS)
		auto vTemp = _mm_cmple_ps(V1, V2);
		return (((_mm_movemask_ps(vTemp) & 7) == 7) != 0);
#else // LM_VMX128_INTRINSICS_
#endif
	}

	inline __m128 LessOrEqualExt(__m128 lhs, __m128 rhs){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vcleq_f32(lhs, rhs);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cmple_ps(lhs, rhs);
#else // _LMVMX128_INTRINSICS_
#endif // _LMVMX128_INTRINSICS_
	}

	inline __m128 LessExt(__m128 lhs, __m128 rhs){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vcltq_f32(lhs, rhs);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cmplt_ps(lhs, rhs);
#else // LM_VMX128_INTRINSICS_
#endif // LM_VMX128_INTRINSICS_
	}

	inline __m128 EqualExt(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vceqq_f32( V1, V2 );
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cmpeq_ps(V1, V2);
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}

	inline __m128 AndInt(__m128 lhs, __m128 rhs){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vandq_u32(lhs, rhs);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_and_ps(lhs, rhs);
#endif
	}

	inline __m128 OrInt(__m128 lhs, __m128 rhs){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vorrq_u32(lhs, rhs);
#elif defined(LM_SSE_INTRINSICS)
		__m128i V = _mm_or_si128(_mm_castps_si128(lhs), _mm_castps_si128(rhs));
		return _mm_castsi128_ps(V);
#endif
	}



	template<uint8 D = 4>
	inline bool EqualInt(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		uint32x4_t vResult = vceqq_u32(V1, V2);
		int8x8x2_t vTemp = vzip_u8(vget_low_u8(vResult), vget_high_u8(vResult));
		vTemp = vzip_u16(vTemp.val[0], vTemp.val[1]);
		return ( vget_lane_u32(vTemp.val[1], 1) == 0xFFFFFFFFU );
#elif defined(LM_SSE_INTRINSICS)
		__m128i vTemp = _mm_cmpeq_epi32(_mm_castps_si128(V1), _mm_castps_si128(V2));
		return ((_mm_movemask_ps(_mm_castsi128_ps(vTemp)) == 0xf) != 0);
#else
#endif
	}

	inline __m128 EqualIntExt(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vceqq_u32(V1, V2);
#elif defined(LM_SSE_INTRINSICS)
		__m128i V = _mm_cmpeq_epi32(_mm_castps_si128(V1), _mm_castps_si128(V2));
		return _mm_castsi128_ps(V);
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}

	template<uint8 D = 4>
	inline bool NotEqualInt(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		uint32x4_t vResult = vceqq_u32(V1, V2);
		int8x8x2_t vTemp = vzip_u8(vget_low_u8(vResult), vget_high_u8(vResult));
		vTemp = vzip_u16(vTemp.val[0], vTemp.val[1]);
		return (vget_lane_u32(vTemp.val[1], 1) != 0xFFFFFFFFU);
#elif defined(LM_SSE_INTRINSICS)
		__m128i vTemp = _mm_cmpeq_epi32(_mm_castps_si128(V1), _mm_castps_si128(V2));
		return ((_mm_movemask_ps(_mm_castsi128_ps(vTemp)) != 0xf) != 0);
#else
#endif
	}

	inline __m128 GreaterExt(__m128 V1, __m128 V2){
#if defined(LM_ARM_NEON_INTRINSICS)
		return vcgtq_f32(V1, V2);
#elif defined(LM_SSE_INTRINSICS)
		return _mm_cmpgt_ps(V1, V2);
#else // LM_ARM_NEON_INTRINSICS
#endif // LM_SSE_INTRINSICS
	}
}

//____m128,std::arrry<__m128,4> operator override
namespace leo{
#if 0
	inline __m128 operator-(__m128 lhs, __m128 rhs){
		return Subtract(lhs, rhs);
	}
#endif
}

//__m128 Trigonometry Function
namespace leo{
	inline __m128 modangle(__m128 Angles){
#if defined(LM_ARM_NEON_INTRINSICS)
		// Modulo the range of the given angles such that -XM_PI <= Angles < XM_PI
		auto vResult = vmulq_f32(Angles, SplatReciprocalTwoPi());
		// Use the inline function due to complexity for rounding
		vResult = XMVectorRound(vResult);
		return vmlsq_f32( Angles, vResult, SplatTwoPi() );
#elif defined(LM_SSE_INTRINSICS)
		// Modulo the range of the given angles such that -XM_PI <= Angles < XM_PI
		auto vResult = _mm_mul_ps(Angles, SplatReciprocalTwoPi());
		// Use the inline function due to complexity for rounding
		vResult = Round(vResult);
		vResult = _mm_mul_ps(vResult, SplatTwoPi());
		vResult = _mm_sub_ps(Angles, vResult);
		return vResult;
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}

	inline __m128 atan(__m128 V){
#if defined(LM_ARM_NEON_INTRINSICS)
		float32x4_t absV = vabsq_f32(V);
		float32x4_t invV = XMVectorReciprocal(V);
		uint32x4_t comp = vcgtq_f32(V, SplatOne());
		uint32x4_t sign = vbslq_f32(comp, SplatOne(), SplatNegativeOne);
		comp = vcleq_f32(absV, SplatOne());
		sign = vbslq_f32(comp, SplatZero(), sign);
		uint32x4_t x = vbslq_f32(comp, V, invV);

		float32x4_t x2 = vmulq_f32(x, x);

		// Compute polynomial approximation
		const auto TC1 = g_XMATanCoefficients1;
		auto vConstants = vdupq_lane_f32(vget_high_f32(TC1), 0);
		auto Result = XM_VMLAQ_LANE_F32(vConstants, x2, vget_high_f32(TC1), 1);

		vConstants = vdupq_lane_f32(vget_low_f32(TC1), 1);
		Result = vmlaq_f32(vConstants, Result, x2);

		vConstants = vdupq_lane_f32(vget_low_f32(TC1), 0);
		Result = vmlaq_f32(vConstants, Result, x2);

		const auto TC0 = g_XMATanCoefficients0;
		vConstants = vdupq_lane_f32(vget_high_f32(TC0), 1);
		Result = vmlaq_f32(vConstants, Result, x2);

		vConstants = vdupq_lane_f32(vget_high_f32(TC0), 0);
		Result = vmlaq_f32(vConstants, Result, x2);

		vConstants = vdupq_lane_f32(vget_low_f32(TC0), 1);
		Result = vmlaq_f32(vConstants, Result, x2);

		vConstants = vdupq_lane_f32(vget_low_f32(TC0), 0);
		Result = vmlaq_f32(vConstants, Result, x2);

		Result = vmlaq_f32(SplatOne(), Result, x2);
		Result = vmulq_f32(Result, x);

		float32x4_t result1 = vmulq_f32(sign, SplatHalfPi());
		result1 = vsubq_f32(result1, Result);

		comp = vceqq_f32(sign, SplatZero());
		Result = vbslq_f32(comp, Result, result1);
		return Result;
#elif defined(LM_SSE_INTRINSICS)
		__m128 absV = Abs(V);
		__m128 invV = _mm_div_ps(SplatOne(), V);
		__m128 comp = _mm_cmpgt_ps(V, SplatOne());
		__m128 select0 = _mm_and_ps(comp, SplatOne());
		__m128 select1 = _mm_andnot_ps(comp, SplatNegativeOne());
		__m128 sign = _mm_or_ps(select0, select1);
		comp = _mm_cmple_ps(absV, SplatOne());
		select0 = _mm_and_ps(comp, SplatZero());
		select1 = _mm_andnot_ps(comp, sign);
		sign = _mm_or_ps(select0, select1);
		select0 = _mm_and_ps(comp, V);
		select1 = _mm_andnot_ps(comp, invV);
		__m128 x = _mm_or_ps(select0, select1);

		__m128 x2 = _mm_mul_ps(x, x);

		// Compute polynomial approximation
		const auto TC1 = load(float4(-0.0752896400f, +0.0429096138f, -0.0161657367f, +0.0028662257f));
		auto vConstants = LM_PERMUTE_PS(TC1, _MM_SHUFFLE(3, 3, 3, 3));
		__m128 Result = _mm_mul_ps(vConstants, x2);

		vConstants = LM_PERMUTE_PS(TC1, _MM_SHUFFLE(2, 2, 2, 2));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);

		vConstants = LM_PERMUTE_PS(TC1, _MM_SHUFFLE(1, 1, 1, 1));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);

		vConstants = LM_PERMUTE_PS(TC1, _MM_SHUFFLE(0, 0, 0, 0));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);

		const auto TC0 = load(float4(-0.3333314528f, +0.1999355085f, -0.1420889944f, +0.1065626393f));
		vConstants = LM_PERMUTE_PS(TC0, _MM_SHUFFLE(3, 3, 3, 3));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);

		vConstants = LM_PERMUTE_PS(TC0, _MM_SHUFFLE(2, 2, 2, 2));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);

		vConstants = LM_PERMUTE_PS(TC0, _MM_SHUFFLE(1, 1, 1, 1));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);

		vConstants = LM_PERMUTE_PS(TC0, _MM_SHUFFLE(0, 0, 0, 0));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);
		Result = _mm_add_ps(Result, SplatOne());
		Result = _mm_mul_ps(Result, x);
		__m128 result1 = _mm_mul_ps(sign, SplatHalfPi());
		result1 = _mm_sub_ps(result1, Result);

		comp = _mm_cmpeq_ps(sign, SplatZero());
		select0 = _mm_and_ps(comp, Result);
		select1 = _mm_andnot_ps(comp, result1);
		Result = _mm_or_ps(select0, select1);
		return Result;
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}

	inline __m128 atan2(__m128 Y, __m128 X){
		// Return the inverse tangent of Y / X in the range of -Pi to Pi with the following exceptions:

		//     Y == 0 and X is Negative         -> Pi with the sign of Y
		//     y == 0 and x is positive         -> 0 with the sign of y
		//     Y != 0 and X == 0                -> Pi / 2 with the sign of Y
		//     Y != 0 and X is Negative         -> atan(y/x) + (PI with the sign of Y)
		//     X == -Infinity and Finite Y      -> Pi with the sign of Y
		//     X == +Infinity and Finite Y      -> 0 with the sign of Y
		//     Y == Infinity and X is Finite    -> Pi / 2 with the sign of Y
		//     Y == Infinity and X == -Infinity -> 3Pi / 4 with the sign of Y
		//     Y == Infinity and X == +Infinity -> Pi / 4 with the sign of Y

		static const auto ATan2Constants = load(float4(LM_PI, LM_HALFPI, LM_QUARPI, LM_PI * 3.0f / 4.0f));

		auto Zero = SplatZero();
		auto ATanResultValid = details::SplatTrueInt();

		auto Pi = SplatX(ATan2Constants);
		auto PiOverTwo = SplatY(ATan2Constants);
		auto PiOverFour = SplatZ(ATan2Constants);
		auto ThreePiOverFour = SplatW(ATan2Constants);

		auto YEqualsZero = EqualExt(Y, Zero);
		auto XEqualsZero = EqualExt(X, Zero);
		auto XIsPositive = AndInt(X, details::SplatNegativeZero());
		XIsPositive = EqualIntExt(XIsPositive, Zero);
		auto YEqualsInfinity = details::IsInfinite(Y);
		auto XEqualsInfinity = details::IsInfinite(X);

		auto YSign = AndInt(Y, details::SplatNegativeZero());
		Pi = OrInt(Pi, YSign);
		PiOverTwo = OrInt(PiOverTwo, YSign);
		PiOverFour = OrInt(PiOverFour, YSign);
		ThreePiOverFour = OrInt(ThreePiOverFour, YSign);

		auto R1 = Select(Pi, YSign, XIsPositive);
		auto R2 = Select(ATanResultValid, PiOverTwo, XEqualsZero);
		auto R3 = Select(R2, R1, YEqualsZero);
		auto R4 = Select(ThreePiOverFour, PiOverFour, XIsPositive);
		auto R5 = Select(PiOverTwo, R4, XEqualsInfinity);
		auto Result = Select(R3, R5, YEqualsInfinity);
		ATanResultValid = EqualIntExt(Result, ATanResultValid);

		auto V = Divide(Y, X);

		auto R0 = atan(V);

		R1 = Select(Pi, Zero, XIsPositive);
		R2 = Add(R0, R1);

		return Select(Result, R2, ATanResultValid);
	}

	inline __m128 sin(__m128 V){
#if defined(LM_ARM_NEON_INTRINSICS)
		// Force the value within the bounds of pi
		auto x = XMVectorModAngles(V);

		// Map in [-pi/2,pi/2] with sin(y) = sin(x).
		uint32x4_t sign = vandq_u32(x, SplatNegativeZero());
		uint32x4_t c = vorrq_u32(g_XMPi, sign);  // pi when x >= 0, -pi when x < 0
		float32x4_t absx = vabsq_f32(x);
		float32x4_t rflx = vsubq_f32(c, x);
		uint32x4_t comp = vcleq_f32(absx, SplatHalfPi());
		x = vbslq_f32(comp, x, rflx);

		float32x4_t x2 = vmulq_f32(x, x);

		// Compute polynomial approximation
		const auto SC1 = g_XMSinCoefficients1;
		const auto SC0 = g_XMSinCoefficients0;
		auto vConstants = vdupq_lane_f32(vget_high_f32(SC0), 1);
		auto Result = XM_VMLAQ_LANE_F32(vConstants, x2, vget_low_f32(SC1), 0);

		vConstants = vdupq_lane_f32(vget_high_f32(SC0), 0);
		Result = vmlaq_f32(vConstants, Result, x2);

		vConstants = vdupq_lane_f32(vget_low_f32(SC0), 1);
		Result = vmlaq_f32(vConstants, Result, x2);

		vConstants = vdupq_lane_f32(vget_low_f32(SC0), 0);
		Result = vmlaq_f32(vConstants, Result, x2);

		Result = vmlaq_f32(g_XMOne, Result, x2);
		Result = vmulq_f32(Result, x);
		return Result;
#elif defined(LM_SSE_INTRINSICS)
		// Force the value within the bounds of pi
		auto x = modangle(V);

		// Map in [-pi/2,pi/2] with sin(y) = sin(x).
		__m128 sign = _mm_and_ps(x, details::SplatNegativeZero());
		__m128 c = _mm_or_ps(SplatPi(), sign);  // pi when x >= 0, -pi when x < 0
		__m128 absx = _mm_andnot_ps(sign, x);  // |x|
		__m128 rflx = _mm_sub_ps(c, x);
		__m128 comp = _mm_cmple_ps(absx, SplatHalfPi());
		__m128 select0 = _mm_and_ps(comp, x);
		__m128 select1 = _mm_andnot_ps(comp, rflx);
		x = _mm_or_ps(select0, select1);

		__m128 x2 = _mm_mul_ps(x, x);

		// Compute polynomial approximation
		const auto SC1 = load(float4(-2.3889859e-08f, -0.16665852f /*Est1*/, +0.0083139502f /*Est2*/, -0.00018524670f /*Est3*/));
		auto vConstants = LM_PERMUTE_PS(SC1, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Result = _mm_mul_ps(vConstants, x2);

		const auto SC0 = load(float4(-0.16666667f, +0.0083333310f, -0.00019840874f, +2.7525562e-06f));
		vConstants = LM_PERMUTE_PS(SC0, _MM_SHUFFLE(3, 3, 3, 3));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);

		vConstants = LM_PERMUTE_PS(SC0, _MM_SHUFFLE(2, 2, 2, 2));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);

		vConstants = LM_PERMUTE_PS(SC0, _MM_SHUFFLE(1, 1, 1, 1));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);

		vConstants = LM_PERMUTE_PS(SC0, _MM_SHUFFLE(0, 0, 0, 0));
		Result = _mm_add_ps(Result, vConstants);
		Result = _mm_mul_ps(Result, x2);
		Result = _mm_add_ps(Result, SplatOne());
		Result = _mm_mul_ps(Result, x);
		return Result;
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}
}

//quaternionslerp
namespace leo{
	inline __m128 QuaternionDot(__m128 Q0, __m128 Q1){
		return Dot<4>(Q0, Q1);
	}

	inline __m128 QuaternionSlerp(__m128 Q0, __m128 Q1, __m128 T){
#if defined(LM_ARM_NEON_INTRINSICS)

		const autoF32 OneMinusEpsilon = { 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f };

		auto CosOmega = XMQuaternionDot(Q0, Q1);

		const auto Zero = Zero();
		auto Control = Less(CosOmega, Zero);
		auto Sign = Select(SplatOne().v, SplatNegativeOne.v, Control);

		CosOmega = Multiply(CosOmega, Sign);

		Control = Less(CosOmega, OneMinusEpsilon);

		auto SinOmega = NegativeMultiplySubtract(CosOmega, CosOmega, SplatOne().v);
		SinOmega = Sqrt(SinOmega);

		auto Omega = ATan2(SinOmega, CosOmega);

		auto SignMask = SplatSignMask();
		auto V01 = ShiftLeft(T, Zero, 2);
		SignMask = ShiftLeft(SignMask, Zero, 3);
		V01 = XorInt(V01, SignMask);
		V01 = Add(g_XMIdentityR0.v, V01);

		auto InvSinOmega = Reciprocal(SinOmega);

		auto S0 = Multiply(V01, Omega);
		S0 = Sin(S0);
		S0 = Multiply(S0, InvSinOmega);

		S0 = Select(V01, S0, Control);

		auto S1 = SplatY(S0);
		S0 = SplatX(S0);

		S1 = Multiply(S1, Sign);

		auto Result = Multiply(Q0, S0);
		Result = MultiplyAdd(Q1, S1, Result);

		return Result;

#elif defined(LM_SSE_INTRINSICS)
		static const auto OneMinusEpsilon = load(float4(1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f));
		static const lalignas(16) uint32 _signmask2[] = { 0x80000000, 0x00000000, 0x00000000, 0x00000000 };
		static const auto SignMask2 = load(*reinterpret_cast<const float4*>(_signmask2));
		static const lalignas(16) uint32 _maskxy[] = { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 };
		static const auto MaskXY = load(*reinterpret_cast<const float4*>(_maskxy));

		auto CosOmega = QuaternionDot(Q0, Q1);

		const auto Zero = SplatZero();
		auto Control = LessExt(CosOmega, Zero);
		auto Sign = Select(SplatOne(), SplatNegativeOne(), Control);

		CosOmega = _mm_mul_ps(CosOmega, Sign);

		Control = LessExt(CosOmega, OneMinusEpsilon);

		auto SinOmega = _mm_mul_ps(CosOmega, CosOmega);
		SinOmega = _mm_sub_ps(SplatOne(), SinOmega);
		SinOmega = _mm_sqrt_ps(SinOmega);

		auto Omega = atan2(SinOmega, CosOmega);

		auto V01 = LM_PERMUTE_PS(T, _MM_SHUFFLE(2, 3, 0, 1));
		V01 = _mm_and_ps(V01, MaskXY);
		V01 = _mm_xor_ps(V01, SignMask2);
		V01 = _mm_add_ps(details::SplatR0(), V01);

		auto S0 = _mm_mul_ps(V01, Omega);
		S0 = sin(S0);
		S0 = _mm_div_ps(S0, SinOmega);

		S0 = Select(V01, S0, Control);

		auto S1 = SplatY(S0);
		S0 = SplatX(S0);

		S1 = _mm_mul_ps(S1, Sign);
		auto Result = _mm_mul_ps(Q0, S0);
		S1 = _mm_mul_ps(S1, Q1);
		Result = _mm_add_ps(Result, S1);
		return Result;
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}

	inline __m128 QuaternionSlerp(__m128 Q0, __m128 Q1, float t){
		return QuaternionSlerp(Q0, Q1, Splat(t));
	}

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
		auto V = Subtract(x, y);
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
		return vgetq_lane_f32(vcombine_f32(Result, Result), 0);
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
		auto V = Subtract(x, y);
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
		return vgetq_lane_f32(vcombine_f32(Result, Result), 0);
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
		auto V = Subtract(x, y);
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
		return vgetq_lane_f32(vcombine_f32(Result, Result), 0);
#elif defined(LM_SSE_INTRINSICS)
		// Perform the dot product on x,y,z and w
		auto vLengthSq = _mm_mul_ps(V, V);
		// vTemp has z and w
		auto vTemp = LM_PERMUTE_PS(vLengthSq, _MM_SHUFFLE(3, 2, 3, 2));
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